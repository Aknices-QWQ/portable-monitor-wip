#define SDL_MAIN_HANDLED

#include "lvgl/lvgl.h"
#include "ecg_monitor_keypad_lvgl.h"

#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    SDL_Scancode scancode;
    ecg_monitor_input_action_t action;
} sdl_key_binding_t;

typedef struct {
    lv_display_t * display;
    lv_obj_t * splash_screen;
    lv_obj_t * progress_bar;
    lv_obj_t * status_label;
    lv_timer_t * progress_timer;
    lv_timer_t * switch_timer;
    lv_timer_t * alive_timer;
    lv_obj_t * alive_label;
    lv_obj_t * alive_dot;
    int progress_value;
    int alive_ticks;
    int space_press_count;
    int beep_samples_left;
    int beep_gap_samples_left;
    int beep_repeat_count;
    int beep_on_samples;
    int beep_total_samples;
    uint32_t space_press_deadline_ms;
    uint32_t next_beep_ms;
    float audio_phase;
    float beep_freq;
    float beep_gain;
    SDL_AudioDeviceID audio_device;
    bool main_loaded;
    bool pending_main_build;
} preview_ctx_t;

static preview_ctx_t g_preview;

static void preview_audio_cb(void * userdata, uint8_t * stream, int len)
{
    preview_ctx_t * ctx = (preview_ctx_t *)userdata;
    int16_t * out = (int16_t *)stream;
    int sample_count = len / (int)sizeof(int16_t);
    const float sample_rate = 48000.0f;

    memset(stream, 0, (size_t)len);

    for(int i = 0; i < sample_count; i++) {
        if(ctx->beep_gap_samples_left > 0) {
            ctx->beep_gap_samples_left--;
            out[i] = 0;
            continue;
        }

        if(ctx->beep_samples_left <= 0) {
            if(ctx->beep_repeat_count > 0) {
                ctx->beep_repeat_count--;
                ctx->beep_gap_samples_left = 2400;
                ctx->beep_samples_left = ctx->beep_on_samples;
            }
            out[i] = 0;
            continue;
        }

        int played_samples = ctx->beep_total_samples - ctx->beep_samples_left;
        int attack_samples = ctx->beep_total_samples / 8;
        int release_samples = ctx->beep_total_samples / 5;
        float env = 1.0f;
        float tone;
        float harmonic;
        float sample_value;

        if(attack_samples < 1) {
            attack_samples = 1;
        }
        if(release_samples < 1) {
            release_samples = 1;
        }

        if(played_samples < attack_samples) {
            env = (float)played_samples / (float)attack_samples;
        }
        else if(ctx->beep_samples_left < release_samples) {
            env = (float)ctx->beep_samples_left / (float)release_samples;
        }

        tone = sinf(ctx->audio_phase);
        harmonic = sinf(ctx->audio_phase * 2.0f) * 0.22f;
        sample_value = (tone + harmonic) * env * ctx->beep_gain;
        out[i] = (int16_t)(sample_value * 14000.0f);
        ctx->audio_phase += (2.0f * 3.14159265f * ctx->beep_freq) / sample_rate;
        if(ctx->audio_phase >= 2.0f * 3.14159265f) {
            ctx->audio_phase -= 2.0f * 3.14159265f;
        }
        ctx->beep_samples_left--;
    }
}

static void preview_start_beep(float freq, int on_samples, int repeat_count, float gain)
{
    if(g_preview.audio_device == 0) {
        return;
    }

    SDL_LockAudioDevice(g_preview.audio_device);
    g_preview.beep_freq = freq;
    g_preview.beep_on_samples = on_samples;
    g_preview.beep_total_samples = on_samples;
    g_preview.beep_samples_left = on_samples;
    g_preview.beep_gap_samples_left = 0;
    g_preview.beep_repeat_count = repeat_count;
    g_preview.beep_gain = gain;
    SDL_UnlockAudioDevice(g_preview.audio_device);
}

static void preview_update_alarm_audio(void)
{
    uint32_t now_ms;

    if(!g_preview.main_loaded || g_preview.audio_device == 0) {
        return;
    }

    now_ms = SDL_GetTicks();

    if(ecg_monitor_keypad_take_prompt_tone_request()) {
        preview_start_beep(1046.0f, 1800, 1, 0.85f);
    }

    if(ecg_monitor_keypad_is_high_priority_alarm_active() && now_ms >= g_preview.next_beep_ms) {
        preview_start_beep(880.0f, 4200, 0, 1.0f);
        g_preview.next_beep_ms = now_ms + 900U;
    }
}

static const sdl_key_binding_t g_key_bindings[] = {
    { SDL_SCANCODE_RETURN,       ECG_INPUT_OK },
    { SDL_SCANCODE_KP_ENTER,     ECG_INPUT_OK },
    { SDL_SCANCODE_ESCAPE,       ECG_INPUT_BACK },
    { SDL_SCANCODE_BACKSPACE,    ECG_INPUT_BACK },
    { SDL_SCANCODE_LEFT,         ECG_INPUT_KNOB_CCW },
    { SDL_SCANCODE_LEFTBRACKET,  ECG_INPUT_KNOB_CCW },
    { SDL_SCANCODE_RIGHT,        ECG_INPUT_KNOB_CW },
    { SDL_SCANCODE_RIGHTBRACKET, ECG_INPUT_KNOB_CW },
    { SDL_SCANCODE_1,            ECG_INPUT_PATIENT },
    { SDL_SCANCODE_2,            ECG_INPUT_ECG },
    { SDL_SCANCODE_3,            ECG_INPUT_NIBP },
    { SDL_SCANCODE_4,            ECG_INPUT_SPO2 },
    { SDL_SCANCODE_5,            ECG_INPUT_ALARM },
    { SDL_SCANCODE_6,            ECG_INPUT_FREEZE },
};

static void preview_poll_keys(void)
{
    static bool prev_state[sizeof(g_key_bindings) / sizeof(g_key_bindings[0])];
    static bool prev_space_pressed;
    int key_count = 0;
    const uint8_t * keyboard_state;
    bool space_pressed = false;
    uint32_t now_ms = SDL_GetTicks();

    if(!g_preview.main_loaded) {
        return;
    }

    SDL_PumpEvents();
    keyboard_state = SDL_GetKeyboardState(&key_count);
    if(keyboard_state == NULL) {
        return;
    }

    if(SDL_SCANCODE_SPACE < key_count) {
        space_pressed = (keyboard_state[SDL_SCANCODE_SPACE] != 0);
    }

    if(now_ms > g_preview.space_press_deadline_ms) {
        g_preview.space_press_count = 0;
    }

    if(space_pressed && !prev_space_pressed) {
        if(g_preview.space_press_count == 0 || now_ms <= g_preview.space_press_deadline_ms) {
            g_preview.space_press_count++;
        }
        else {
            g_preview.space_press_count = 1;
        }

        g_preview.space_press_deadline_ms = now_ms + 1200U;

        if(g_preview.space_press_count >= 3) {
            g_preview.space_press_count = 0;
            g_preview.space_press_deadline_ms = 0;
            ecg_monitor_keypad_open_test_menu();
        }
    }

    prev_space_pressed = space_pressed;

    for(size_t i = 0; i < (sizeof(g_key_bindings) / sizeof(g_key_bindings[0])); i++) {
        SDL_Scancode scancode = g_key_bindings[i].scancode;
        bool pressed = (scancode < key_count) ? (keyboard_state[scancode] != 0) : false;

        if(pressed && !prev_state[i]) {
            ecg_monitor_keypad_handle_input(g_key_bindings[i].action);
        }

        prev_state[i] = pressed;
    }
}

static lv_obj_t * create_center_column(lv_obj_t * parent)
{
    lv_obj_t * col = lv_obj_create(parent);

    lv_obj_remove_style_all(col);
    lv_obj_set_size(col, 820, 420);
    lv_obj_center(col);
    lv_obj_set_layout(col, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_pad_row(col, 16, 0);

    return col;
}

static void preview_alive_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    g_preview.alive_ticks++;

    if(g_preview.alive_dot) {
        lv_color_t color = (g_preview.alive_ticks & 1) ? lv_color_hex(0x79d07f) : lv_color_hex(0x24422a);
        lv_obj_set_style_bg_color(g_preview.alive_dot, color, 0);
    }

    if(g_preview.alive_label) {
        lv_label_set_text_fmt(g_preview.alive_label, "Main loop alive  %d", g_preview.alive_ticks);
    }
}

static void preview_cleanup_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    if(g_preview.alive_timer) {
        lv_timer_del(g_preview.alive_timer);
        g_preview.alive_timer = NULL;
    }

    g_preview.alive_label = NULL;
    g_preview.alive_dot = NULL;
}

static void splash_progress_cb(lv_timer_t * timer)
{
    static const char * const steps[] = {
        "Init display bus",
        "Create monitor layers",
        "Allocate waveform buffers",
        "Bind keypad zones",
        "Open main workspace"
    };
    size_t step_index;

    LV_UNUSED(timer);

    if(g_preview.progress_value >= 100) {
        lv_timer_del(g_preview.progress_timer);
        g_preview.progress_timer = NULL;
        return;
    }

    g_preview.progress_value += 7;
    if(g_preview.progress_value > 100) {
        g_preview.progress_value = 100;
    }

    lv_bar_set_value(g_preview.progress_bar, g_preview.progress_value, LV_ANIM_ON);
    step_index = (size_t)((g_preview.progress_value / 20) % (sizeof(steps) / sizeof(steps[0])));
    lv_label_set_text(g_preview.status_label, steps[step_index]);
}

static void splash_switch_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    g_preview.pending_main_build = true;
    lv_label_set_text(g_preview.status_label, "Open main workspace");
}

static void create_splash_screen(lv_display_t * display)
{
    lv_obj_t * col;
    lv_obj_t * logo;
    lv_obj_t * title;
    lv_obj_t * subtitle;
    lv_obj_t * footer;
    lv_obj_t * signature;

    g_preview.display = display;
    g_preview.splash_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(g_preview.splash_screen);
    lv_obj_set_style_bg_color(g_preview.splash_screen, lv_color_hex(0x081109), 0);
    lv_obj_set_style_bg_grad_color(g_preview.splash_screen, lv_color_hex(0x13261a), 0);
    lv_obj_set_style_bg_grad_dir(g_preview.splash_screen, LV_GRAD_DIR_VER, 0);

    col = create_center_column(g_preview.splash_screen);

    logo = lv_obj_create(col);
    lv_obj_remove_style_all(logo);
    lv_obj_set_size(logo, 148, 148);
    lv_obj_set_style_radius(logo, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(logo, lv_color_hex(0x10361d), 0);
    lv_obj_set_style_border_width(logo, 2, 0);
    lv_obj_set_style_border_color(logo, lv_color_hex(0x75d08f), 0);

    title = lv_label_create(logo);
    lv_label_set_text(title, "PM");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xcff5d9), 0);
    lv_obj_center(title);

    title = lv_label_create(col);
    lv_label_set_text(title, "PortaMon SDL Preview");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe6f6ea), 0);

    subtitle = lv_label_create(col);
    lv_label_set_text(subtitle, "LVGL startup path check");
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x9ac4a5), 0);

    g_preview.progress_bar = lv_bar_create(col);
    lv_obj_set_size(g_preview.progress_bar, 620, 18);
    lv_bar_set_range(g_preview.progress_bar, 0, 100);
    lv_bar_set_value(g_preview.progress_bar, 8, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_preview.progress_bar, lv_color_hex(0x132018), LV_PART_MAIN);
    lv_obj_set_style_radius(g_preview.progress_bar, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_preview.progress_bar, lv_color_hex(0x76d698), LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_preview.progress_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    g_preview.status_label = lv_label_create(col);
    lv_label_set_text(g_preview.status_label, "Boot monitor renderer");
    lv_obj_set_style_text_font(g_preview.status_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_preview.status_label, lv_color_hex(0xa6cbb1), 0);

    footer = lv_label_create(g_preview.splash_screen);
    lv_label_set_text(footer, "If this animates, lv_timer_handler is alive");
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(footer, lv_color_hex(0x69806f), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -28);

    signature = lv_label_create(g_preview.splash_screen);
    lv_label_set_text(signature, "WANGGAOLI 2026");
    lv_obj_set_style_text_font(signature, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(signature, lv_color_hex(0x8fb49a), 0);
    lv_obj_align(signature, LV_ALIGN_BOTTOM_MID, 0, -8);

    lv_screen_load(g_preview.splash_screen);
    lv_refr_now(display);

    g_preview.progress_value = 8;
    g_preview.progress_timer = lv_timer_create(splash_progress_cb, 110, NULL);
    g_preview.switch_timer = lv_timer_create(splash_switch_cb, 1600, NULL);
}

static void build_main_ui_from_splash(void)
{
    if(g_preview.switch_timer) {
        lv_timer_del(g_preview.switch_timer);
        g_preview.switch_timer = NULL;
    }

    if(g_preview.progress_timer) {
        lv_timer_del(g_preview.progress_timer);
        g_preview.progress_timer = NULL;
    }

    if(g_preview.splash_screen == NULL) {
        return;
    }

    lv_obj_clean(g_preview.splash_screen);
    lv_obj_remove_style_all(g_preview.splash_screen);
    lv_obj_set_style_bg_color(g_preview.splash_screen, lv_color_hex(0x040704), 0);
    lv_screen_load(g_preview.splash_screen);

    ecg_monitor_keypad_create(g_preview.splash_screen);

    lv_refr_now(g_preview.display);

    g_preview.main_loaded = true;
    g_preview.pending_main_build = false;
}

int main(void)
{
#ifndef WIN32
    setenv("DBUS_FATAL_WARNINGS", "0", 1);
#endif

    lv_display_t * display;
    SDL_AudioSpec want;
    SDL_AudioSpec have;

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");

    lv_init();
    display = lv_sdl_window_create(1280, 720);
    if(display == NULL) {
        fprintf(stderr, "lv_sdl_window_create failed: %s\n", SDL_GetError());
        return 1;
    }

    lv_sdl_window_set_title(display, "ECG Monitor LVGL SDL Preview");
    lv_sdl_window_set_zoom(display, 1.25f);

    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();

    if(SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            fprintf(stderr, "SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n", SDL_GetError());
        }
    }

    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = preview_audio_cb;
    want.userdata = &g_preview;
    g_preview.audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                                                 SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
                                                 SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    if(g_preview.audio_device != 0) {
        fprintf(stderr, "SDL audio ready: freq=%d channels=%u samples=%u\n",
                have.freq, (unsigned)have.channels, (unsigned)have.samples);
        SDL_PauseAudioDevice(g_preview.audio_device, 0);
    }
    else {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    }

    create_splash_screen(display);
    printf("ECG SDL preview\n");

    while(1) {
        lv_timer_handler();

        if(g_preview.pending_main_build && !g_preview.main_loaded) {
            build_main_ui_from_splash();
        }

        preview_poll_keys();
        preview_update_alarm_audio();
        usleep(5000);
    }

    return 0;
}
