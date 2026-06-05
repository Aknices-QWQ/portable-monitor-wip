#include "lvgl/lvgl.h"
#include "lv_drivers/display/sunxifb.h"
#include "ecg_monitor_keypad_lvgl.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define GPIO_COL4_PD15 111
#define GPIO_COL3_PD16 112
#define GPIO_COL2_PD17 113
#define GPIO_COL1_PD18 114
#define GPIO_ROW2_PD20 116
#define GPIO_ROW1_PD21 117
#define GPIO_ENC_B_PG13 205
#define GPIO_ENC_A_PG16 208
#define ENCODER_TRANSITIONS_PER_DETENT 4

typedef struct {
    int pin;
    int fd;
} gpio_pin_t;

typedef struct {
    bool enabled;
    gpio_pin_t rows[2];
    gpio_pin_t cols[4];
    gpio_pin_t enc_a;
    gpio_pin_t enc_b;
    uint8_t key_history[8];
    bool key_stable[8];
    uint8_t encoder_prev_state;
    int encoder_accum;
} board_input_t;

static const ecg_monitor_input_action_t g_key_actions[8] = {
    ECG_INPUT_OK,
    ECG_INPUT_BACK,
    ECG_INPUT_PATIENT,
    ECG_INPUT_ECG,
    ECG_INPUT_NIBP,
    ECG_INPUT_SPO2,
    ECG_INPUT_ALARM,
    ECG_INPUT_FREEZE
};

uint32_t custom_tick_get(void)
{
    static uint64_t start_ms;
    struct timeval tv_now;
    uint64_t now_ms;

    if(start_ms == 0U) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = ((uint64_t)tv_start.tv_sec * 1000000ULL + (uint64_t)tv_start.tv_usec) / 1000ULL;
    }

    gettimeofday(&tv_now, NULL);
    now_ms = ((uint64_t)tv_now.tv_sec * 1000000ULL + (uint64_t)tv_now.tv_usec) / 1000ULL;

    return (uint32_t)(now_ms - start_ms);
}

static bool path_exists(const char * path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int write_text_file(const char * path, const char * text)
{
    int fd = open(path, O_WRONLY);
    ssize_t ret;

    if(fd < 0) {
        return -1;
    }

    ret = write(fd, text, strlen(text));
    close(fd);
    return (ret < 0) ? -1 : 0;
}

static int export_gpio_if_needed(int pin)
{
    char path[128];
    char num[16];

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", pin);
    if(path_exists(path)) {
        return 0;
    }

    snprintf(num, sizeof(num), "%d", pin);
    if(write_text_file("/sys/class/gpio/export", num) == 0) {
        return 0;
    }

    if(errno == EBUSY) {
        return 0;
    }

    return -1;
}

static int gpio_open_value_fd(int pin, int flags)
{
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    return open(path, flags);
}

static int gpio_init_input(gpio_pin_t * gpio, int pin)
{
    char path[128];

    gpio->pin = pin;
    gpio->fd = -1;

    if(export_gpio_if_needed(pin) != 0) {
        return -1;
    }

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    if(write_text_file(path, "in") != 0) {
        return -1;
    }

    gpio->fd = gpio_open_value_fd(pin, O_RDONLY);
    return (gpio->fd >= 0) ? 0 : -1;
}

static int gpio_init_output(gpio_pin_t * gpio, int pin, bool high)
{
    char path[128];

    gpio->pin = pin;
    gpio->fd = -1;

    if(export_gpio_if_needed(pin) != 0) {
        return -1;
    }

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    if(write_text_file(path, high ? "high" : "low") != 0) {
        return -1;
    }

    gpio->fd = gpio_open_value_fd(pin, O_WRONLY);
    return (gpio->fd >= 0) ? 0 : -1;
}

static int gpio_write_level(gpio_pin_t * gpio, bool high)
{
    const char level = high ? '1' : '0';

    if(gpio->fd < 0) {
        return -1;
    }

    if(lseek(gpio->fd, 0, SEEK_SET) < 0) {
        return -1;
    }

    return (write(gpio->fd, &level, 1) == 1) ? 0 : -1;
}

static int gpio_read_level(gpio_pin_t * gpio)
{
    char level;

    if(gpio->fd < 0) {
        return -1;
    }

    if(lseek(gpio->fd, 0, SEEK_SET) < 0) {
        return -1;
    }

    if(read(gpio->fd, &level, 1) != 1) {
        return -1;
    }

    return (level == '0') ? 0 : 1;
}

static void board_input_close(board_input_t * input)
{
    uint8_t i;

    for(i = 0; i < 2; i++) {
        if(input->rows[i].fd >= 0) close(input->rows[i].fd);
    }

    for(i = 0; i < 4; i++) {
        if(input->cols[i].fd >= 0) close(input->cols[i].fd);
    }

    if(input->enc_a.fd >= 0) close(input->enc_a.fd);
    if(input->enc_b.fd >= 0) close(input->enc_b.fd);

    memset(input, 0, sizeof(*input));
}

static bool board_input_init(board_input_t * input)
{
    static const int row_pins[2] = { GPIO_ROW2_PD20, GPIO_ROW1_PD21 };
    static const int col_pins[4] = { GPIO_COL1_PD18, GPIO_COL2_PD17, GPIO_COL3_PD16, GPIO_COL4_PD15 };
    uint8_t i;

    memset(input, 0, sizeof(*input));
    input->enc_a.fd = -1;
    input->enc_b.fd = -1;
    for(i = 0; i < 2; i++) input->rows[i].fd = -1;
    for(i = 0; i < 4; i++) input->cols[i].fd = -1;

    for(i = 0; i < 2; i++) {
        if(gpio_init_output(&input->rows[i], row_pins[i], true) != 0) {
            fprintf(stderr, "failed to init row gpio %d\n", row_pins[i]);
            board_input_close(input);
            return false;
        }
    }

    for(i = 0; i < 4; i++) {
        if(gpio_init_input(&input->cols[i], col_pins[i]) != 0) {
            fprintf(stderr, "failed to init col gpio %d\n", col_pins[i]);
            board_input_close(input);
            return false;
        }
    }

    if(gpio_init_input(&input->enc_a, GPIO_ENC_A_PG16) != 0 ||
       gpio_init_input(&input->enc_b, GPIO_ENC_B_PG13) != 0) {
        fprintf(stderr, "failed to init encoder gpio\n");
        board_input_close(input);
        return false;
    }

    input->enabled = true;
    input->encoder_prev_state = (uint8_t)((gpio_read_level(&input->enc_a) << 1) | gpio_read_level(&input->enc_b));
    return true;
}

static void board_scan_matrix(board_input_t * input, bool raw_pressed[8])
{
    uint8_t row;
    uint8_t col;

    for(row = 0; row < 2; row++) {
        gpio_write_level(&input->rows[0], row != 0U);
        gpio_write_level(&input->rows[1], row != 1U);
        usleep(120);

        for(col = 0; col < 4; col++) {
            int level = gpio_read_level(&input->cols[col]);
            uint8_t key_index = (row == 0U) ? col : (7U - col);
            raw_pressed[key_index] = (level == 0);
        }
    }

    gpio_write_level(&input->rows[0], true);
    gpio_write_level(&input->rows[1], true);
}

static void board_poll_keys(board_input_t * input)
{
    bool raw_pressed[8] = { false };
    uint8_t i;

    board_scan_matrix(input, raw_pressed);

    for(i = 0; i < 8; i++) {
        input->key_history[i] = (uint8_t)((input->key_history[i] << 1) | (raw_pressed[i] ? 1U : 0U));

        if(input->key_history[i] == 0xFFU && !input->key_stable[i]) {
            input->key_stable[i] = true;
            ecg_monitor_keypad_handle_input(g_key_actions[i]);
        }
        else if(input->key_history[i] == 0x00U && input->key_stable[i]) {
            input->key_stable[i] = false;
        }
    }
}

static void board_poll_encoder(board_input_t * input)
{
    static const int8_t quad_table[16] = {
        0, -1,  1,  0,
        1,  0,  0, -1,
       -1,  0,  0,  1,
        0,  1, -1,  0
    };
    int a = gpio_read_level(&input->enc_a);
    int b = gpio_read_level(&input->enc_b);
    uint8_t state;
    int8_t delta;

    if(a < 0 || b < 0) {
        return;
    }

    state = (uint8_t)((a << 1) | b);
    delta = quad_table[(input->encoder_prev_state << 2) | state];
    input->encoder_prev_state = state;

    if(delta == 0) {
        return;
    }

    input->encoder_accum += delta;
    if(input->encoder_accum >= ENCODER_TRANSITIONS_PER_DETENT) {
        input->encoder_accum = 0;
        ecg_monitor_keypad_handle_input(ECG_INPUT_KNOB_CW);
    }
    else if(input->encoder_accum <= -ENCODER_TRANSITIONS_PER_DETENT) {
        input->encoder_accum = 0;
        ecg_monitor_keypad_handle_input(ECG_INPUT_KNOB_CCW);
    }
}

static void board_input_poll(board_input_t * input)
{
    if(!input->enabled) {
        return;
    }

    board_poll_keys(input);
    board_poll_encoder(input);
}

int main(void)
{
    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;
    static uint32_t width;
    static uint32_t height;
    board_input_t input;
    lv_color_t * buf;

    lv_init();
    sunxifb_init(LV_DISP_ROT_NONE);
    sunxifb_get_sizes(&width, &height);

#ifdef USE_SUNXIFB_DIRECT_MODE
    buf = (lv_color_t *)sunxifb_get_buf();
#else
    buf = (lv_color_t *)sunxifb_alloc(width * height * sizeof(lv_color_t), "ecg_monitor");
#endif

    if(buf == NULL) {
        fprintf(stderr, "sunxifb_alloc failed\n");
        sunxifb_exit();
        return 1;
    }

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, width * height);

    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = sunxifb_flush;
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;
    disp_drv.sw_rotate = 1;

#ifdef USE_SUNXIFB_DIRECT_MODE
    disp_drv.direct_mode = 1;
    disp_drv.full_refresh = 1;
#endif

    lv_disp_drv_register(&disp_drv);

    if(!board_input_init(&input)) {
        fprintf(stderr, "board input disabled; UI will still render\n");
        memset(&input, 0, sizeof(input));
    }

    ecg_monitor_keypad_load();

    while(1) {
        board_input_poll(&input);
        lv_timer_handler();
        usleep(5000);
    }

    board_input_close(&input);
    return 0;
}
