#include "ecg_monitor_keypad_lvgl.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

LV_FONT_DECLARE(ecg_font_zh_16);
LV_FONT_DECLARE(ecg_font_zh_20);
LV_FONT_DECLARE(ecg_font_zh_28);
LV_FONT_DECLARE(ecg_font_en_16);
LV_FONT_DECLARE(ecg_font_en_20);
LV_FONT_DECLARE(ecg_font_en_22);
LV_FONT_DECLARE(ecg_font_en_36);
LV_FONT_DECLARE(ecg_font_en_48);
LV_FONT_DECLARE(ecg_font_en_56);
LV_FONT_DECLARE(ecg_font_en_64);

#ifdef ECG_SDL_PREVIEW
#define ECG_LOGICAL_W             1280
#define ECG_LOGICAL_H             720
#define ECG_TOPBAR_H              92
#define ECG_MODEBAR_H             68
#define ECG_DOCK_H                84
#define ECG_PARAM_PANEL_W         404
#define ECG_CHART_POINT_COUNT     180
#define ECG_TOPBAR_PAD_X          28
#define ECG_TOPBAR_GAP            20
#define ECG_MODE_CHIP_PAD_X       16
#define ECG_PANEL_BLOCK_PAD_X     22
#define ECG_PANEL_BLOCK_PAD_Y     14
#define ECG_BADGE_PAD_X           12
#define ECG_BADGE_PAD_Y           4
#define ECG_PULSE_DOT_SIZE        18
#define ECG_MENU_PANEL_W          560
#define ECG_MENU_PANEL_H          392
#define ECG_MENU_PANEL_MARGIN     26
#define ECG_MENU_ROW_H            58
#define ECG_FREEZE_OVERLAY_Y      24
#define ECG_CONTROL_CARD_W        178
#define ECG_CONTROL_CARD_H        126
#define ECG_LEFT_ZONE_W           430
#define ECG_LEFT_CARDS_H          130
#define ECG_RIGHT_ZONE_W          820
#define ECG_RIGHT_ZONE_H          176
#define ECG_RIGHT_CARDS_H         128
#define ECG_SHORTCUT_CARD_W       136
#define ECG_SHORTCUT_CARD_H       118
#else
#define ECG_LOGICAL_W             1920
#define ECG_LOGICAL_H             1080
#define ECG_TOPBAR_H              84
#define ECG_MODEBAR_H             60
#define ECG_DOCK_H                80
#define ECG_PARAM_PANEL_W         420
#define ECG_CHART_POINT_COUNT     300
#define ECG_TOPBAR_PAD_X          30
#define ECG_TOPBAR_GAP            20
#define ECG_MODE_CHIP_PAD_X       18
#define ECG_PANEL_BLOCK_PAD_X     24
#define ECG_PANEL_BLOCK_PAD_Y     10
#define ECG_BADGE_PAD_X           12
#define ECG_BADGE_PAD_Y           4
#define ECG_PULSE_DOT_SIZE        18
#define ECG_MENU_PANEL_W          620
#define ECG_MENU_PANEL_H          360
#define ECG_MENU_PANEL_MARGIN     28
#define ECG_MENU_ROW_H            58
#define ECG_FREEZE_OVERLAY_Y      18
#define ECG_CONTROL_CARD_W        208
#define ECG_CONTROL_CARD_H        104
#define ECG_LEFT_ZONE_W           620
#define ECG_LEFT_CARDS_H          112
#define ECG_RIGHT_ZONE_W          1180
#define ECG_RIGHT_ZONE_H          150
#define ECG_RIGHT_CARDS_H         104
#define ECG_SHORTCUT_CARD_W       176
#define ECG_SHORTCUT_CARD_H       96
#endif
#define ECG_WAVE_ROW_COUNT        5
#define ECG_SAMPLE_PERIOD_MS      35
#define ECG_INFO_PERIOD_MS        250
#define ECG_MENU_ITEM_COUNT       5

#define ECG_BG_ROOT               0x000000
#define ECG_BG_SURFACE            0x050505
#define ECG_BG_SURFACE_ALT        0x090909
#define ECG_BG_OVERLAY            0x101010
#define ECG_BG_FOCUS              0x141414
#define ECG_BORDER_SUBTLE         0x1a1a1a
#define ECG_BORDER_SOFT           0x262626
#define ECG_TEXT_PRIMARY          0xe2e2e2
#define ECG_TEXT_SECONDARY        0xb0b0b0
#define ECG_TEXT_TERTIARY         0x7e7e7e

#if LVGL_VERSION_MAJOR >= 9
#define ECG_DISP_T                lv_display_t
#define ECG_DISP_GET_DEFAULT()    lv_display_get_default()
#define ECG_DISP_SET_ROTATION     lv_display_set_rotation
#define ECG_ROTATION_90           LV_DISPLAY_ROTATION_90
#define ECG_SCREEN_LOAD           lv_screen_load
#define ECG_SCREEN_ACTIVE()       lv_screen_active()
#define ECG_SET_STYLE_SIZE(obj, w, h, sel) lv_obj_set_style_size((obj), (w), (h), (sel))
#else
#define ECG_DISP_T                lv_disp_t
#define ECG_DISP_GET_DEFAULT()    lv_disp_get_default()
#define ECG_DISP_SET_ROTATION     lv_disp_set_rotation
#define ECG_ROTATION_90           LV_DISP_ROT_90
#define ECG_SCREEN_LOAD           lv_scr_load
#define ECG_SCREEN_ACTIVE()       lv_scr_act()
#define ECG_SET_STYLE_SIZE(obj, w, h, sel) LV_UNUSED(h); lv_obj_set_style_size((obj), (w), (sel))
#endif

#ifndef LV_MIN
#define LV_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef LV_MAX
#define LV_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef ECG_PI
#ifdef LV_PI
#define ECG_PI ((float)LV_PI)
#else
#define ECG_PI 3.14159265f
#endif
#endif

#define ECG_FONT_XS              (&ecg_font_zh_16)
#define ECG_FONT_SM              (&ecg_font_zh_20)
#define ECG_FONT_MD              (&ecg_font_zh_20)
#define ECG_FONT_LG              (&ecg_font_zh_28)
#define ECG_FONT_XL              (&ecg_font_en_48)
#define ECG_FONT_EN_XS           (&ecg_font_en_16)
#define ECG_FONT_EN_SM           (&ecg_font_en_20)
#define ECG_FONT_EN_MD           (&ecg_font_en_22)
#define ECG_FONT_EN_LG           (&ecg_font_en_36)
#define ECG_FONT_EN_XL           (&ecg_font_en_48)
#define ECG_FONT_EN_XXL          (&ecg_font_en_56)
#define ECG_FONT_EN_XXXL         (&ecg_font_en_64)

typedef enum {
    ECG_SECTION_PATIENT = 0,
    ECG_SECTION_ECG,
    ECG_SECTION_NIBP,
    ECG_SECTION_SPO2,
    ECG_SECTION_ALARM,
    ECG_SECTION_TEST,
    ECG_SECTION_COUNT
} ecg_section_t;

typedef enum {
    ECG_ITEM_ENUM = 0,
    ECG_ITEM_INT,
    ECG_ITEM_ACTION
} ecg_menu_item_kind_t;

typedef struct {
    const char * label;
    ecg_menu_item_kind_t kind;
    const char * const * options;
    uint8_t option_count;
    int min_value;
    int max_value;
    int step;
    const char * unit;
} ecg_menu_item_def_t;

typedef struct {
    const char * lead_name;
    lv_color_t color;
    lv_obj_t * chart;
    lv_chart_series_t * series;
} ecg_wave_row_t;

typedef struct {
    lv_obj_t * screen;
    lv_obj_t * pulse_dot;
    lv_obj_t * clock_label;
    lv_obj_t * alarm_badge;
    lv_obj_t * patient_badge;
    lv_obj_t * message_label;

    lv_obj_t * hr_value;
    lv_obj_t * nibp_sys_value;
    lv_obj_t * nibp_dia_value;
    lv_obj_t * nibp_map_value;
    lv_obj_t * nibp_time_label;
    lv_obj_t * spo2_value;
    lv_obj_t * pr_value;
    lv_obj_t * pi_value;

    lv_obj_t * mode_patient_label;
    lv_obj_t * mode_ecg_label;
    lv_obj_t * mode_nibp_label;
    lv_obj_t * mode_spo2_label;
    lv_obj_t * mode_alarm_label;

    lv_obj_t * menu_panel;
    lv_obj_t * menu_title;
    lv_obj_t * menu_hint;
    lv_obj_t * menu_rows[ECG_MENU_ITEM_COUNT];
    lv_obj_t * menu_item_labels[ECG_MENU_ITEM_COUNT];
    lv_obj_t * menu_value_labels[ECG_MENU_ITEM_COUNT];

    lv_obj_t * shortcut_cards[6];
    lv_obj_t * shortcut_value_labels[6];
    lv_obj_t * shortcut_title_labels[6];
    lv_obj_t * freeze_overlay;
    lv_timer_t * sample_timer;
    lv_timer_t * info_timer;

    uint32_t elapsed_ms;
    bool pulse_on;
    bool menu_open;
    bool edit_mode;
    bool freeze_active;
    bool test_wave_fast;
    bool test_wave_pause;
    bool test_asystole;

    ecg_section_t active_section;
    uint8_t focus_index;
    int settings[ECG_SECTION_COUNT][ECG_MENU_ITEM_COUNT];
    uint32_t alarm_silence_remaining_s;
    uint32_t alarm_silence_tick_ms;
    uint32_t nibp_cycle_elapsed_ms;
    time_t nibp_last_measure_time;

    ecg_wave_row_t waves[ECG_WAVE_ROW_COUNT];
} ecg_monitor_ctx_t;

static ecg_monitor_ctx_t g_ctx;
static bool g_preview_enable_shortcut_refresh;
static bool g_high_priority_alarm_active;
static bool g_prompt_tone_request_pending;

static const char * const g_patient_profile_opts[] = { "成人", "儿童", "新生儿" };
static const char * const g_patient_pace_opts[] = { "关闭", "开启" };
static const char * const g_patient_screen_opts[] = { "常规", "创伤", "夜间" };
static const char * const g_ecg_lead_opts[] = { "I", "II", "III", "aVR" };
static const char * const g_ecg_gain_opts[] = { "5", "10", "20" };
static const char * const g_ecg_speed_opts[] = { "12.5", "25", "50" };
static const char * const g_ecg_filter_opts[] = { "诊断", "监护", "手术" };
static const char * const g_nibp_mode_opts[] = { "手动", "自动", "连续" };
static const char * const g_nibp_interval_opts[] = { "5", "10", "15", "30" };
static const char * const g_nibp_cuff_opts[] = { "成人", "儿童" };
static const char * const g_spo2_tone_opts[] = { "关闭", "开启" };
static const char * const g_spo2_avg_opts[] = { "4", "8", "16" };
static const char * const g_spo2_sens_opts[] = { "正常", "高灵敏" };
static const char * const g_spo2_pr_src_opts[] = { "血氧", "心电" };
static const char * const g_alarm_silence_opts[] = { "关闭", "120", "300" };
static const char * const g_test_toggle_opts[] = { "关闭", "开启" };

static const ecg_menu_item_def_t g_menu_defs[ECG_SECTION_COUNT][ECG_MENU_ITEM_COUNT] = {
    [ECG_SECTION_PATIENT] = {
        { "病人类型", ECG_ITEM_ENUM, g_patient_profile_opts, 3, 0, 0, 0, NULL },
        { "床号",     ECG_ITEM_INT,  NULL, 0, 1, 12, 1, NULL },
        { "起搏",     ECG_ITEM_ENUM, g_patient_pace_opts, 2, 0, 0, 0, NULL },
        { "界面",     ECG_ITEM_ENUM, g_patient_screen_opts, 3, 0, 0, 0, NULL },
    },
    [ECG_SECTION_ECG] = {
        { "导联",   ECG_ITEM_ENUM, g_ecg_lead_opts, 4, 0, 0, 0, NULL },
        { "增益",   ECG_ITEM_ENUM, g_ecg_gain_opts, 3, 0, 0, 0, "mm/mV" },
        { "速度",   ECG_ITEM_ENUM, g_ecg_speed_opts, 3, 0, 0, 0, "mm/s" },
        { "滤波",   ECG_ITEM_ENUM, g_ecg_filter_opts, 3, 0, 0, 0, NULL },
    },
    [ECG_SECTION_NIBP] = {
        { "测量",     ECG_ITEM_ACTION, NULL, 0, 0, 0, 0, NULL },
        { "模式",     ECG_ITEM_ENUM, g_nibp_mode_opts, 3, 0, 0, 0, NULL },
        { "间隔",     ECG_ITEM_ENUM, g_nibp_interval_opts, 4, 0, 0, 0, "min" },
        { "袖带",     ECG_ITEM_ENUM, g_nibp_cuff_opts, 2, 0, 0, 0, NULL },
    },
    [ECG_SECTION_SPO2] = {
        { "音调",       ECG_ITEM_ENUM, g_spo2_tone_opts, 2, 0, 0, 0, NULL },
        { "平均",       ECG_ITEM_ENUM, g_spo2_avg_opts, 3, 0, 0, 0, "s" },
        { "灵敏度",     ECG_ITEM_ENUM, g_spo2_sens_opts, 2, 0, 0, 0, NULL },
        { "脉率来源",   ECG_ITEM_ENUM, g_spo2_pr_src_opts, 2, 0, 0, 0, NULL },
    },
    [ECG_SECTION_ALARM] = {
        { "静音",     ECG_ITEM_ENUM, g_alarm_silence_opts, 3, 0, 0, 0, "s" },
        { "音量",     ECG_ITEM_INT,  NULL, 0, 1, 5, 1, NULL },
        { "心率上限", ECG_ITEM_INT,  NULL, 0, 100, 180, 5, "bpm" },
        { "血氧下限", ECG_ITEM_INT,  NULL, 0, 85, 95, 1, "%" },
        { "",         ECG_ITEM_ACTION, NULL, 0, 0, 0, 0, NULL },
    },
    [ECG_SECTION_TEST] = {
        { "模拟波形", ECG_ITEM_ENUM, g_test_toggle_opts, 2, 0, 0, 0, NULL },
        { "波形加速", ECG_ITEM_ENUM, g_test_toggle_opts, 2, 0, 0, 0, NULL },
        { "暂停波形", ECG_ITEM_ENUM, g_test_toggle_opts, 2, 0, 0, 0, NULL },
        { "心脏停跳", ECG_ITEM_ENUM, g_test_toggle_opts, 2, 0, 0, 0, NULL },
        { "测试提示音", ECG_ITEM_ACTION, NULL, 0, 0, 0, 0, NULL },
    },
};

static int32_t ecg_round_to_i32(float v)
{
    return (int32_t)((v >= 0.0f) ? (v + 0.5f) : (v - 0.5f));
}

static float ecg_wave_sample(float seconds, uint8_t lead_index)
{
    float p = fmodf(seconds * 1.2f, 1.0f);
    float v = 0.0f;

    if(p < 0.05f) v = sinf((p / 0.05f) * ECG_PI) * 0.08f;
    else if(p < 0.12f) v = -sinf(((p - 0.05f) / 0.07f) * ECG_PI) * 0.04f;
    else if(p < 0.18f) v = sinf(((p - 0.16f) / 0.02f) * ECG_PI) * 0.14f;
    else if(p < 0.20f) v = -sinf(((p - 0.18f) / 0.02f) * ECG_PI) * 0.07f;
    else if(p < 0.22f) v = -sinf(((p - 0.20f) / 0.02f) * ECG_PI) * 0.18f;
    else if(p < 0.24f) v = sinf(((p - 0.22f) / 0.02f) * ECG_PI) * 1.00f;
    else if(p < 0.26f) v = sinf(((p - 0.24f) / 0.02f) * ECG_PI) * -0.28f;
    else if(p < 0.30f) v = -sinf(((p - 0.26f) / 0.04f) * ECG_PI) * 0.05f;
    else if(p < 0.46f) v = sinf(((p - 0.36f) / 0.10f) * ECG_PI) * 0.16f;

    static const float lead_gain[4] = { 1.00f, 0.86f, 0.56f, -0.42f };
    return v * lead_gain[lead_index % 4];
}

static float spo2_wave_sample(float seconds)
{
    float p = fmodf(seconds * 1.2f, 1.0f);

    if(p < 0.15f) return sinf((p / 0.15f) * ECG_PI) * 0.90f;
    if(p < 0.25f) return sinf(((p - 0.15f) / 0.10f) * ECG_PI) * -0.14f;
    return 0.0f;
}

static void set_label_text(lv_obj_t * label, const char * fmt, ...)
{
    char buf[96];
    va_list ap;

    va_start(ap, fmt);
    lv_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    lv_label_set_text(label, buf);
}

static const char * get_section_name(ecg_section_t section)
{
    static const char * const names[ECG_SECTION_COUNT] = {
        "病人",
        "心电",
        "血压",
        "血氧",
        "报警",
        "测试"
    };

    return names[section];
}

static lv_color_t get_section_color(ecg_section_t section)
{
    static const lv_color_t colors[ECG_SECTION_COUNT] = {
        LV_COLOR_MAKE(0xe0, 0xdb, 0x8a),
        LV_COLOR_MAKE(0x00, 0xff, 0x88),
        LV_COLOR_MAKE(0xff, 0xaa, 0x44),
        LV_COLOR_MAKE(0x00, 0xcc, 0xff),
        LV_COLOR_MAKE(0xff, 0x66, 0x66),
        LV_COLOR_MAKE(0xc8, 0xc8, 0xc8)
    };

    return colors[section];
}

static uint8_t get_menu_item_count(ecg_section_t section)
{
    return (section == ECG_SECTION_TEST) ? 5U : 4U;
}

static const ecg_menu_item_def_t * get_menu_item_def(ecg_section_t section, uint8_t item_index)
{
    return &g_menu_defs[section][item_index];
}

static int get_menu_value(ecg_section_t section, uint8_t item_index)
{
    return g_ctx.settings[section][item_index];
}

static void set_menu_value(ecg_section_t section, uint8_t item_index, int value)
{
    g_ctx.settings[section][item_index] = value;
}

static void update_clock_label(void)
{
    time_t now = time(NULL);
    struct tm * local_tm = localtime(&now);

    if(g_ctx.clock_label == NULL) {
        return;
    }

    if(local_tm) {
        set_label_text(g_ctx.clock_label, "%02d:%02d:%02d",
                       local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec);
    }
    else {
        lv_label_set_text(g_ctx.clock_label, "00:00:00");
    }
}

static void set_message_text(const char * fmt, ...)
{
    char buf[128];
    va_list ap;

    va_start(ap, fmt);
    lv_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if(g_ctx.message_label) {
        lv_label_set_text(g_ctx.message_label, buf);
    }
}

static bool ecg_text_has_non_ascii(const char * text)
{
    const unsigned char * p = (const unsigned char *)text;

    if(text == NULL) {
        return false;
    }

    while(*p != '\0') {
        if(*p & 0x80U) {
            return true;
        }
        p++;
    }

    return false;
}

static void format_menu_value(ecg_section_t section, uint8_t item_index, char * buf, size_t buf_size)
{
    const ecg_menu_item_def_t * def = get_menu_item_def(section, item_index);
    int value = get_menu_value(section, item_index);

    if(def->kind == ECG_ITEM_ACTION) {
        if(section == ECG_SECTION_NIBP && item_index == 0U) {
            lv_snprintf(buf, buf_size, "%s", value ? "停止测量" : "开始测量");
            return;
        }

        lv_snprintf(buf, buf_size, "%s", "执行");
        return;
    }

    if(def->kind == ECG_ITEM_ENUM && def->options && value >= 0 && value < def->option_count) {
        if(def->unit) {
            lv_snprintf(buf, buf_size, "%s%s%s", def->options[value], def->unit[0] ? " " : "", def->unit);
        }
        else {
            lv_snprintf(buf, buf_size, "%s", def->options[value]);
        }
        return;
    }

    if(def->unit) {
        lv_snprintf(buf, buf_size, "%d %s", value, def->unit);
    }
    else {
        lv_snprintf(buf, buf_size, "%d", value);
    }
}

static void create_mode_chip(lv_obj_t * parent, lv_obj_t ** out_label)
{
    lv_obj_t * label = lv_label_create(parent);

    lv_obj_set_style_text_font(label, ECG_FONT_XS, 0);
    lv_obj_set_style_pad_left(label, ECG_MODE_CHIP_PAD_X, 0);
    lv_obj_set_style_pad_right(label, ECG_MODE_CHIP_PAD_X, 0);
    lv_obj_set_style_border_side(label, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_width(label, 1, 0);
    lv_obj_set_style_border_color(label, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x4b6f4b), 0);

    *out_label = label;
}

static lv_obj_t * create_panel_block(lv_obj_t * parent, lv_color_t bg)
{
    lv_obj_t * block = lv_obj_create(parent);

    lv_obj_remove_style_all(block);
    lv_obj_set_height(block, LV_PCT(25));
    lv_obj_set_width(block, LV_PCT(100));
    lv_obj_set_style_bg_color(block, bg, 0);
    lv_obj_set_style_bg_opa(block, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(block, ECG_PANEL_BLOCK_PAD_X, 0);
    lv_obj_set_style_pad_right(block, ECG_PANEL_BLOCK_PAD_X, 0);
    lv_obj_set_style_pad_top(block, ECG_PANEL_BLOCK_PAD_Y, 0);
    lv_obj_set_style_pad_bottom(block, ECG_PANEL_BLOCK_PAD_Y, 0);
    lv_obj_set_style_border_side(block, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(block, 1, 0);
    lv_obj_set_style_border_color(block, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_layout(block, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(block, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(block, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    return block;
}

static void create_hr_block(lv_obj_t * parent)
{
    lv_obj_t * block = create_panel_block(parent, lv_color_hex(ECG_BG_SURFACE));
    lv_obj_t * header = lv_label_create(block);
    lv_obj_t * value_row = lv_obj_create(block);
    lv_obj_t * limits = lv_label_create(block);
    lv_obj_t * rhythm = lv_label_create(block);
    lv_obj_t * rl_drv = lv_label_create(block);
    lv_obj_t * unit = lv_label_create(value_row);

    lv_label_set_text(header, "HR");
    lv_obj_set_style_text_font(header, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0x00ff88), 0);

    lv_obj_remove_style_all(value_row);
    lv_obj_set_width(value_row, LV_PCT(100));
    lv_obj_set_height(value_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(value_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(value_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    g_ctx.hr_value = lv_label_create(value_row);
    lv_label_set_text(g_ctx.hr_value, "72");
    lv_obj_set_style_text_font(g_ctx.hr_value, ECG_FONT_EN_XXXL, 0);
    lv_obj_set_style_text_color(g_ctx.hr_value, lv_color_hex(0x00ff88), 0);

    lv_label_set_text(unit, "bpm");
    lv_obj_set_style_text_font(unit, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(unit, lv_color_hex(0x00dd77), 0);
    lv_obj_set_style_pad_left(unit, 6, 0);

    lv_label_set_text(limits, "HI 100\nLO 50");
    lv_obj_set_style_text_font(limits, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(limits, lv_color_hex(0x00ff88), 0);
    lv_obj_set_style_text_align(limits, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(limits, LV_ALIGN_TOP_RIGHT, -6, 6);
    lv_obj_add_flag(limits, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_FLOATING);

    lv_label_set_text(rhythm, "Sinus Rhythm");
    lv_obj_set_style_text_font(rhythm, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(rhythm, lv_color_hex(0x6ba56b), 0);

    lv_label_set_text(rl_drv, "RL DRV OK");
    lv_obj_set_style_text_font(rl_drv, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(rl_drv, lv_color_hex(0x007a33), 0);
}

static void create_nibp_block(lv_obj_t * parent)
{
    lv_obj_t * block = create_panel_block(parent, lv_color_hex(ECG_BG_SURFACE));
    lv_obj_t * header = lv_label_create(block);
    lv_obj_t * value_row = lv_obj_create(block);
    lv_obj_t * sep = lv_label_create(value_row);
    lv_obj_t * limits = lv_label_create(block);
    lv_obj_t * map_row = lv_obj_create(block);
    lv_obj_t * map_text = lv_label_create(map_row);

    lv_label_set_text(header, "NIBP");
    lv_obj_set_style_text_font(header, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0xffaa44), 0);

    lv_obj_remove_style_all(value_row);
    lv_obj_set_width(value_row, LV_PCT(100));
    lv_obj_set_height(value_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(value_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(value_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    g_ctx.nibp_sys_value = lv_label_create(value_row);
    lv_label_set_text(g_ctx.nibp_sys_value, "118");
    lv_obj_set_style_text_font(g_ctx.nibp_sys_value, ECG_FONT_EN_XXL, 0);
    lv_obj_set_style_text_color(g_ctx.nibp_sys_value, lv_color_hex(0xffaa44), 0);

    lv_label_set_text(sep, "/");
    lv_obj_set_style_text_font(sep, ECG_FONT_EN_MD, 0);
    lv_obj_set_style_text_color(sep, lv_color_hex(0xffaa44), 0);
    lv_obj_set_style_pad_left(sep, 4, 0);
    lv_obj_set_style_pad_right(sep, 4, 0);

    g_ctx.nibp_dia_value = lv_label_create(value_row);
    lv_label_set_text(g_ctx.nibp_dia_value, "76");
    lv_obj_set_style_text_font(g_ctx.nibp_dia_value, ECG_FONT_EN_XL, 0);
    lv_obj_set_style_text_color(g_ctx.nibp_dia_value, lv_color_hex(0xffaa44), 0);

    lv_label_set_text(limits, "HI 160\nLO 60");
    lv_obj_set_style_text_font(limits, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(limits, lv_color_hex(0xffaa44), 0);
    lv_obj_set_style_text_align(limits, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(limits, LV_ALIGN_TOP_RIGHT, -6, 6);
    lv_obj_add_flag(limits, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_FLOATING);

    lv_obj_remove_style_all(map_row);
    lv_obj_set_width(map_row, LV_PCT(100));
    lv_obj_set_height(map_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(map_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(map_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(map_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_label_set_text(map_text, "MAP:");
    lv_obj_set_style_text_font(map_text, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(map_text, lv_color_hex(0xbb7a22), 0);

    g_ctx.nibp_map_value = lv_label_create(map_row);
    lv_label_set_text(g_ctx.nibp_map_value, "90");
    lv_obj_set_style_text_font(g_ctx.nibp_map_value, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(g_ctx.nibp_map_value, lv_color_hex(0xffaa44), 0);
    lv_obj_set_style_pad_left(g_ctx.nibp_map_value, 6, 0);

    lv_obj_t * unit = lv_label_create(map_row);
    lv_label_set_text(unit, "mmHg");
    lv_obj_set_style_text_font(unit, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(unit, lv_color_hex(0xbb7a22), 0);
    lv_obj_set_style_pad_left(unit, 6, 0);

    g_ctx.nibp_time_label = lv_label_create(block);
    lv_label_set_text(g_ctx.nibp_time_label, "Last --:--");
    lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
    lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0x5a3a08), 0);
}

static void create_spo2_block(lv_obj_t * parent)
{
    lv_obj_t * block = create_panel_block(parent, lv_color_hex(ECG_BG_SURFACE));
    lv_obj_t * header = lv_label_create(block);
    lv_obj_t * value_row = lv_obj_create(block);
    lv_obj_t * limits = lv_label_create(block);
    lv_obj_t * pr_row = lv_obj_create(block);
    lv_obj_t * pr_text = lv_label_create(pr_row);
    lv_obj_t * unit = lv_label_create(value_row);

    lv_label_set_text(header, "SpO2");
    lv_obj_set_style_text_font(header, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0x00ccff), 0);

    lv_obj_remove_style_all(value_row);
    lv_obj_set_width(value_row, LV_PCT(100));
    lv_obj_set_height(value_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(value_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(value_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    g_ctx.spo2_value = lv_label_create(value_row);
    lv_label_set_text(g_ctx.spo2_value, "98");
    lv_obj_set_style_text_font(g_ctx.spo2_value, ECG_FONT_EN_XXXL, 0);
    lv_obj_set_style_text_color(g_ctx.spo2_value, lv_color_hex(0x00ccff), 0);

    lv_label_set_text(unit, "%");
    lv_obj_set_style_text_font(unit, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(unit, lv_color_hex(0x009dcc), 0);
    lv_obj_set_style_pad_left(unit, 6, 0);

    lv_label_set_text(limits, "HI 100\nLO 90");
    lv_obj_set_style_text_font(limits, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(limits, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_text_align(limits, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(limits, LV_ALIGN_TOP_RIGHT, -6, 6);
    lv_obj_add_flag(limits, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_FLOATING);

    lv_obj_remove_style_all(pr_row);
    lv_obj_set_width(pr_row, LV_PCT(100));
    lv_obj_set_height(pr_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(pr_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(pr_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pr_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_label_set_text(pr_text, "PR:");
    lv_obj_set_style_text_font(pr_text, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(pr_text, lv_color_hex(0x6aa7ba), 0);

    g_ctx.pr_value = lv_label_create(pr_row);
    lv_label_set_text(g_ctx.pr_value, "71");
    lv_obj_set_style_text_font(g_ctx.pr_value, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(g_ctx.pr_value, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_pad_left(g_ctx.pr_value, 6, 0);

    lv_obj_t * bpm = lv_label_create(pr_row);
    lv_label_set_text(bpm, "bpm");
    lv_obj_set_style_text_font(bpm, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(bpm, lv_color_hex(0x6aa7ba), 0);
    lv_obj_set_style_pad_left(bpm, 6, 0);
}

static void create_pi_block(lv_obj_t * parent)
{
    lv_obj_t * block = create_panel_block(parent, lv_color_hex(ECG_BG_SURFACE));
    lv_obj_t * header = lv_label_create(block);
    lv_obj_t * value_row = lv_obj_create(block);
    lv_obj_t * unit = lv_label_create(value_row);
    lv_obj_t * sub = lv_label_create(block);

    lv_obj_set_style_border_side(block, LV_BORDER_SIDE_NONE, 0);

    lv_label_set_text(header, "PI");
    lv_obj_set_style_text_font(header, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0x0088bb), 0);

    lv_obj_remove_style_all(value_row);
    lv_obj_set_width(value_row, LV_PCT(100));
    lv_obj_set_height(value_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(value_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(value_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    g_ctx.pi_value = lv_label_create(value_row);
    lv_label_set_text(g_ctx.pi_value, "2.8");
    lv_obj_set_style_text_font(g_ctx.pi_value, ECG_FONT_EN_XXL, 0);
    lv_obj_set_style_text_color(g_ctx.pi_value, lv_color_hex(0x0088bb), 0);

    lv_label_set_text(unit, "%");
    lv_obj_set_style_text_font(unit, ECG_FONT_EN_SM, 0);
    lv_obj_set_style_text_color(unit, lv_color_hex(0x0088bb), 0);
    lv_obj_set_style_pad_left(unit, 6, 0);

    lv_label_set_text(sub, "Perfusion");
    lv_obj_set_style_text_font(sub, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(sub, lv_color_hex(0x1a5a7a), 0);
}

static void create_wave_rows(lv_obj_t * parent)
{
    static const char * lead_names[ECG_WAVE_ROW_COUNT] = { "I", "II", "III", "aVR", "SpO2" };
    static const lv_color_t lead_colors[ECG_WAVE_ROW_COUNT] = {
        LV_COLOR_MAKE(0x00, 0xff, 0x88),
        LV_COLOR_MAKE(0x00, 0xe4, 0x7a),
        LV_COLOR_MAKE(0x00, 0xc8, 0x68),
        LV_COLOR_MAKE(0x00, 0xac, 0x58),
        LV_COLOR_MAKE(0x00, 0xcc, 0xff),
    };

    for(uint8_t i = 0; i < ECG_WAVE_ROW_COUNT; i++) {
        lv_obj_t * row = lv_obj_create(parent);
        lv_obj_t * lead_label = lv_label_create(row);
        lv_obj_t * chart = lv_chart_create(row);

        g_ctx.waves[i].lead_name = lead_names[i];
        g_ctx.waves[i].color = lead_colors[i];
        g_ctx.waves[i].chart = chart;

        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_flex_grow(row, 1);
        lv_obj_set_style_bg_color(row, lv_color_hex(ECG_BG_ROOT), 0);
        lv_obj_set_style_border_side(row, i == (ECG_WAVE_ROW_COUNT - 1U) ? LV_BORDER_SIDE_NONE : LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(ECG_BORDER_SUBTLE), 0);
        lv_obj_set_style_pad_all(row, 0, 0);

        lv_label_set_text(lead_label, lead_names[i]);
        lv_obj_set_style_text_font(lead_label, ECG_FONT_SM, 0);
        lv_obj_set_style_text_color(lead_label, lead_colors[i], 0);
        lv_obj_align(lead_label, LV_ALIGN_TOP_LEFT, 10, 6);
        lv_obj_add_flag(lead_label, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_FLOATING);

        lv_obj_set_size(chart, LV_PCT(100), LV_PCT(100));
        lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(chart, lv_color_hex(ECG_BG_ROOT), 0);
        lv_obj_set_style_border_width(chart, 0, 0);
        lv_obj_set_style_pad_all(chart, 0, 0);
        lv_obj_set_style_line_width(chart, 1, LV_PART_MAIN);
        lv_obj_set_style_line_color(chart, lv_color_hex(ECG_BORDER_SOFT), LV_PART_MAIN);
        lv_obj_set_style_line_opa(chart, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
        ECG_SET_STYLE_SIZE(chart, 0, 0, LV_PART_INDICATOR);
        lv_obj_clear_flag(chart, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(chart, ECG_CHART_POINT_COUNT);
        lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
        lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
        lv_chart_set_div_line_count(chart, 6, 20);

        g_ctx.waves[i].series = lv_chart_add_series(chart, lead_colors[i], LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(chart, g_ctx.waves[i].series, 0);
    }
}

static void create_preview_stub(lv_obj_t * parent, const char * title, const char * body, lv_color_t accent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_t * title_label = lv_label_create(panel);
    lv_obj_t * body_label = lv_label_create(panel);

    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(panel, lv_color_hex(ECG_BG_SURFACE), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(panel, 28, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel, 14, 0);

    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, ECG_FONT_LG, 0);
    lv_obj_set_style_text_color(title_label, accent, 0);

    lv_label_set_text(body_label, body);
    lv_obj_set_style_text_font(body_label, ECG_FONT_MD, 0);
    lv_obj_set_style_text_color(body_label, lv_color_hex(ECG_TEXT_SECONDARY), 0);
    lv_obj_set_style_text_align(body_label, LV_TEXT_ALIGN_CENTER, 0);
}

static void style_shortcut_card(lv_obj_t * card, lv_color_t accent)
{
    lv_obj_set_style_bg_color(card, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, accent, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 10, 0);
}

static lv_obj_t * create_control_card(lv_obj_t * parent, const char * title, const char * body, lv_color_t accent)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_t * title_label = lv_label_create(card);
    lv_obj_t * body_label = lv_label_create(card);

    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, ECG_CONTROL_CARD_W, ECG_CONTROL_CARD_H);
    style_shortcut_card(card, accent);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(title_label, accent, 0);

    lv_label_set_text(body_label, body);
    lv_obj_set_style_text_font(body_label, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(body_label, lv_color_hex(ECG_TEXT_SECONDARY), 0);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body_label, LV_PCT(100));

    return card;
}

static lv_obj_t * create_dock_zone_panel(lv_obj_t * parent, lv_coord_t width)
{
    lv_obj_t * panel = lv_obj_create(parent);

    lv_obj_remove_style_all(panel);
    lv_obj_set_height(panel, LV_PCT(100));
    if(width > 0) {
        lv_obj_set_width(panel, width);
    }
    lv_obj_set_style_bg_color(panel, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_pad_all(panel,
#ifdef ECG_SDL_PREVIEW
                             12
#else
                             16
#endif
    , 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(panel,
#ifdef ECG_SDL_PREVIEW
                             10
#else
                             14
#endif
    , 0);

    return panel;
}

static void create_dock_placeholder(lv_obj_t * parent, const char * title, const char * body, lv_color_t accent)
{
    lv_obj_t * title_label = lv_label_create(parent);
    lv_obj_t * body_label = lv_label_create(parent);

    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, ECG_FONT_MD, 0);
    lv_obj_set_style_text_color(title_label, accent, 0);

    lv_label_set_text(body_label, body);
    lv_obj_set_style_text_font(body_label, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(body_label, lv_color_hex(ECG_TEXT_SECONDARY), 0);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body_label, LV_PCT(100));
}

static void create_function_zone_compact(lv_obj_t * parent)
{
    lv_obj_t * line1 = lv_label_create(parent);
    lv_obj_t * line2 = lv_label_create(parent);

    lv_label_set_text(line1, "FUNC  L3 PATIENT   L4 ECG   L5 NIBP");
    lv_obj_set_style_text_font(line1, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(line1, lv_color_hex(ECG_TEXT_PRIMARY), 0);

    lv_label_set_text(line2, "      L6 SpO2   L7 ALARM   L8 FREEZE");
    lv_obj_set_style_text_font(line2, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(line2, lv_color_hex(ECG_TEXT_SECONDARY), 0);
}

static void create_compact_control_zone(lv_obj_t * parent)
{
    lv_obj_t * line = lv_label_create(parent);

    lv_label_set_text(line, "CTRL  L1 OK   L2 BACK   KNOB NAV");
    lv_obj_set_style_text_font(line, ECG_FONT_SM, 0);
    lv_obj_set_style_text_color(line, lv_color_hex(ECG_TEXT_PRIMARY), 0);
}

static void create_control_zone(lv_obj_t * parent)
{
    lv_obj_t * title = lv_label_create(parent);
    lv_obj_t * row = lv_obj_create(parent);

    lv_label_set_text(title, "CONTROL ZONE  |  Left1 Left2 Encoder");
    lv_obj_set_style_text_font(title, ECG_FONT_MD, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe7f3e7), 0);
    lv_obj_set_width(title, LV_PCT(100));
    lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);

    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), ECG_LEFT_CARDS_H);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 10, 0);

    lv_obj_t * ok_card = create_control_card(row, "L1 OK", "Open section\nConfirm edit", lv_color_hex(0x79d07f));
    lv_obj_t * back_card = create_control_card(row, "L2 BACK", "Close editor\nHide menu", lv_color_hex(0xe0db8a));
    lv_obj_t * knob_card = create_control_card(row, "ENCODER", "Rotate focus or value\nUse with L1", lv_color_hex(0x8bd8f0));

    lv_obj_set_width(ok_card, LV_PCT(32));
    lv_obj_set_flex_grow(ok_card, 1);
    lv_obj_set_width(back_card, LV_PCT(32));
    lv_obj_set_flex_grow(back_card, 1);
    lv_obj_set_width(knob_card, LV_PCT(32));
    lv_obj_set_flex_grow(knob_card, 1);
}

static void update_freeze_overlay(void)
{
    if(g_ctx.freeze_overlay == NULL) {
        return;
    }

    if(g_ctx.freeze_active) {
        lv_obj_clear_flag(g_ctx.freeze_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(g_ctx.freeze_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

static void update_modebar_labels(void)
{
    char buf[96];

    if(g_ctx.mode_patient_label == NULL ||
       g_ctx.patient_badge == NULL ||
       g_ctx.mode_ecg_label == NULL ||
       g_ctx.mode_nibp_label == NULL ||
       g_ctx.mode_spo2_label == NULL ||
       g_ctx.mode_alarm_label == NULL ||
       g_ctx.alarm_badge == NULL) {
        return;
    }

    format_menu_value(ECG_SECTION_PATIENT, 0, buf, sizeof(buf));
    set_label_text(g_ctx.mode_patient_label, "PAT %s  BED %d", buf, get_menu_value(ECG_SECTION_PATIENT, 1));

    set_label_text(g_ctx.patient_badge, "%s", g_patient_profile_opts[get_menu_value(ECG_SECTION_PATIENT, 0)]);

    set_label_text(g_ctx.mode_ecg_label, "ECG %s  %s mm/mV  %s mm/s",
                   g_ecg_lead_opts[get_menu_value(ECG_SECTION_ECG, 0)],
                   g_ecg_gain_opts[get_menu_value(ECG_SECTION_ECG, 1)],
                   g_ecg_speed_opts[get_menu_value(ECG_SECTION_ECG, 2)]);

    set_label_text(g_ctx.mode_nibp_label, "NIBP %s  %s min  %s",
                   g_nibp_mode_opts[get_menu_value(ECG_SECTION_NIBP, 1)],
                   g_nibp_interval_opts[get_menu_value(ECG_SECTION_NIBP, 2)],
                   g_nibp_cuff_opts[get_menu_value(ECG_SECTION_NIBP, 3)]);

    set_label_text(g_ctx.mode_spo2_label, "SpO2 Tone %s  Avg %ss",
                   g_spo2_tone_opts[get_menu_value(ECG_SECTION_SPO2, 0)],
                   g_spo2_avg_opts[get_menu_value(ECG_SECTION_SPO2, 1)]);

    if(g_ctx.alarm_silence_remaining_s > 0U) {
        g_high_priority_alarm_active = false;
        set_label_text(g_ctx.alarm_badge, "报警静音 %us", (unsigned)g_ctx.alarm_silence_remaining_s);
        lv_obj_set_style_text_color(g_ctx.alarm_badge, lv_color_hex(0xffcc55), 0);
        set_label_text(g_ctx.mode_alarm_label, "静音 %us  音量 %d  心率高限 %d",
                       (unsigned)g_ctx.alarm_silence_remaining_s,
                       get_menu_value(ECG_SECTION_ALARM, 1),
                       get_menu_value(ECG_SECTION_ALARM, 2));
    }
    else {
        if(g_ctx.test_asystole) {
            g_high_priority_alarm_active = true;
            lv_label_set_text(g_ctx.alarm_badge, "心脏停跳报警");
            lv_obj_set_style_text_color(g_ctx.alarm_badge, lv_color_hex(0xff6666), 0);
            lv_obj_set_style_bg_color(g_ctx.alarm_badge, lv_color_hex(0x240808), 0);
            set_label_text(g_ctx.mode_alarm_label, "心脏停跳  音量 %d  需立即处理",
                           get_menu_value(ECG_SECTION_ALARM, 1));
        }
        else if(get_menu_value(ECG_SECTION_ALARM, 2) <= 110) {
            g_high_priority_alarm_active = true;
            lv_label_set_text(g_ctx.alarm_badge, "高优先级报警");
            lv_obj_set_style_text_color(g_ctx.alarm_badge, lv_color_hex(0xff6666), 0);
            lv_obj_set_style_bg_color(g_ctx.alarm_badge, lv_color_hex(0x240808), 0);
            set_label_text(g_ctx.mode_alarm_label, "心率过高风险  音量 %d  血氧低限 %d",
                           get_menu_value(ECG_SECTION_ALARM, 1),
                           get_menu_value(ECG_SECTION_ALARM, 3));
        }
        else {
            g_high_priority_alarm_active = false;
            lv_label_set_text(g_ctx.alarm_badge, "报警监护开启");
            lv_obj_set_style_text_color(g_ctx.alarm_badge, lv_color_hex(0x00aa44), 0);
            lv_obj_set_style_bg_color(g_ctx.alarm_badge, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
            set_label_text(g_ctx.mode_alarm_label, "报警音量 %d  心率高限 %d  血氧低限 %d",
                           get_menu_value(ECG_SECTION_ALARM, 1),
                           get_menu_value(ECG_SECTION_ALARM, 2),
                           get_menu_value(ECG_SECTION_ALARM, 3));
        }
    }
}

bool ecg_monitor_keypad_is_high_priority_alarm_active(void)
{
    return g_high_priority_alarm_active;
}

bool ecg_monitor_keypad_take_prompt_tone_request(void)
{
    bool pending = g_prompt_tone_request_pending;
    g_prompt_tone_request_pending = false;
    return pending;
}

static void update_shortcut_cards(void)
{
    if(!g_preview_enable_shortcut_refresh) {
        return;
    }

    if(g_ctx.shortcut_title_labels[0] == NULL || g_ctx.shortcut_value_labels[0] == NULL ||
       g_ctx.shortcut_title_labels[1] == NULL || g_ctx.shortcut_value_labels[1] == NULL ||
       g_ctx.shortcut_title_labels[2] == NULL || g_ctx.shortcut_value_labels[2] == NULL ||
       g_ctx.shortcut_title_labels[3] == NULL || g_ctx.shortcut_value_labels[3] == NULL ||
       g_ctx.shortcut_title_labels[4] == NULL || g_ctx.shortcut_value_labels[4] == NULL ||
       g_ctx.shortcut_title_labels[5] == NULL || g_ctx.shortcut_value_labels[5] == NULL) {
        return;
    }

    set_label_text(g_ctx.shortcut_title_labels[0], "L3 PATIENT");
    set_label_text(g_ctx.shortcut_value_labels[0], "%s  Bed %d",
                   g_patient_profile_opts[get_menu_value(ECG_SECTION_PATIENT, 0)],
                   get_menu_value(ECG_SECTION_PATIENT, 1));

    set_label_text(g_ctx.shortcut_title_labels[1], "L4 ECG");
    set_label_text(g_ctx.shortcut_value_labels[1], "%s  %s mm/s  %s",
                   g_ecg_lead_opts[get_menu_value(ECG_SECTION_ECG, 0)],
                   g_ecg_speed_opts[get_menu_value(ECG_SECTION_ECG, 2)],
                   g_ecg_filter_opts[get_menu_value(ECG_SECTION_ECG, 3)]);

    set_label_text(g_ctx.shortcut_title_labels[2], "L5 NIBP");
    set_label_text(g_ctx.shortcut_value_labels[2], "%s  %s",
                   g_nibp_mode_opts[get_menu_value(ECG_SECTION_NIBP, 1)],
                   get_menu_value(ECG_SECTION_NIBP, 0) ? "Running" : "Idle");

    set_label_text(g_ctx.shortcut_title_labels[3], "L6 SpO2");
    set_label_text(g_ctx.shortcut_value_labels[3], "Tone %s  Avg %ss",
                   g_spo2_tone_opts[get_menu_value(ECG_SECTION_SPO2, 0)],
                   g_spo2_avg_opts[get_menu_value(ECG_SECTION_SPO2, 1)]);

    set_label_text(g_ctx.shortcut_title_labels[4], "L7 ALARM");
    if(g_ctx.alarm_silence_remaining_s > 0U) {
        set_label_text(g_ctx.shortcut_value_labels[4], "Sil %us  Vol %d",
                       (unsigned)g_ctx.alarm_silence_remaining_s,
                       get_menu_value(ECG_SECTION_ALARM, 1));
    }
    else {
        set_label_text(g_ctx.shortcut_value_labels[4], "Vol %d  SpO2 %d%%",
                       get_menu_value(ECG_SECTION_ALARM, 1),
                       get_menu_value(ECG_SECTION_ALARM, 3));
    }

    set_label_text(g_ctx.shortcut_title_labels[5], "L8 FREEZE");
    lv_label_set_text(g_ctx.shortcut_value_labels[5], g_ctx.freeze_active ? "Waveforms held" : "Live waveforms");
}

static void update_menu_panel(void)
{
    if(g_ctx.menu_panel == NULL) {
        return;
    }

    if(!g_ctx.menu_open) {
        lv_obj_add_flag(g_ctx.menu_panel, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_color_t accent = get_section_color(g_ctx.active_section);
    lv_obj_clear_flag(g_ctx.menu_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_border_color(g_ctx.menu_panel, accent, 0);
    lv_obj_set_style_bg_color(g_ctx.menu_panel, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_shadow_color(g_ctx.menu_panel, accent, 0);
    lv_obj_set_style_shadow_width(g_ctx.menu_panel, 20, 0);
    lv_obj_set_style_shadow_opa(g_ctx.menu_panel, LV_OPA_20, 0);

    set_label_text(g_ctx.menu_title, "%s设置", get_section_name(g_ctx.active_section));
    lv_obj_set_style_text_color(g_ctx.menu_title, accent, 0);

    if(g_ctx.active_section == ECG_SECTION_TEST) {
        lv_label_set_text(g_ctx.menu_hint, "旋钮: 移动焦点    L1: 切换/执行    L2: 关闭菜单");
    }
    else if(g_ctx.edit_mode) {
        lv_label_set_text(g_ctx.menu_hint, "旋钮: 调整数值    L1: 确认    L2: 退出编辑");
    }
    else {
        lv_label_set_text(g_ctx.menu_hint, "旋钮: 移动焦点    L1: 编辑/执行    L2: 关闭菜单");
    }

    uint8_t item_count = get_menu_item_count(g_ctx.active_section);

    for(uint8_t i = 0; i < ECG_MENU_ITEM_COUNT; i++) {
        if(i >= item_count) {
            lv_obj_add_flag(g_ctx.menu_rows[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        lv_obj_clear_flag(g_ctx.menu_rows[i], LV_OBJ_FLAG_HIDDEN);
        const ecg_menu_item_def_t * def = get_menu_item_def(g_ctx.active_section, i);
        char value_buf[64];
        bool is_focus = (i == g_ctx.focus_index);
        bool is_edit = is_focus && g_ctx.edit_mode && def->kind != ECG_ITEM_ACTION;

        format_menu_value(g_ctx.active_section, i, value_buf, sizeof(value_buf));
        set_label_text(g_ctx.menu_item_labels[i], "%c %s", is_focus ? (is_edit ? '*' : '>') : ' ', def->label);
        lv_label_set_text(g_ctx.menu_value_labels[i], value_buf);
        lv_obj_set_style_text_font(g_ctx.menu_value_labels[i],
                                   ecg_text_has_non_ascii(value_buf) ? ECG_FONT_SM : ECG_FONT_EN_SM,
                                   0);

        lv_obj_set_style_bg_opa(g_ctx.menu_rows[i], is_focus ? LV_OPA_COVER : LV_OPA_0, 0);
        lv_obj_set_style_bg_color(g_ctx.menu_rows[i], is_focus ? lv_color_hex(ECG_BG_FOCUS) : lv_color_hex(ECG_BG_SURFACE_ALT), 0);
        lv_obj_set_style_border_width(g_ctx.menu_rows[i], is_focus ? 1 : 0, 0);
        lv_obj_set_style_border_color(g_ctx.menu_rows[i], accent, 0);
        lv_obj_set_style_text_color(g_ctx.menu_item_labels[i], is_focus ? accent : lv_color_hex(ECG_TEXT_SECONDARY), 0);
        lv_obj_set_style_text_color(g_ctx.menu_value_labels[i], is_focus ? lv_color_white() : lv_color_hex(ECG_TEXT_PRIMARY), 0);
    }
}

static void refresh_ui(void)
{
    update_modebar_labels();
    update_shortcut_cards();
    update_menu_panel();
    update_freeze_overlay();
}

static void open_section(ecg_section_t section)
{
    g_ctx.active_section = section;
    g_ctx.menu_open = true;
    g_ctx.edit_mode = false;
    g_ctx.focus_index = 0;
    set_message_text("%s区域已打开", get_section_name(section));
    refresh_ui();
}

void ecg_monitor_keypad_open_test_menu(void)
{
    open_section(ECG_SECTION_TEST);
    set_message_text("测试菜单已打开");
}

static void start_nibp_cycle(bool start)
{
    set_menu_value(ECG_SECTION_NIBP, 0, start ? 1 : 0);
    g_ctx.nibp_cycle_elapsed_ms = 0;

    if(start) {
        set_message_text("无创血压开始测量");
    }
    else {
        g_ctx.nibp_last_measure_time = time(NULL);
        set_message_text("无创血压停止测量");
    }

    refresh_ui();
}

static void apply_live_setting_effects(ecg_section_t section, uint8_t item_index)
{
    if(section == ECG_SECTION_ALARM && item_index == 0U) {
        static const uint16_t silence_seconds[] = { 0U, 120U, 300U };
        int silence_idx = get_menu_value(ECG_SECTION_ALARM, 0);
        g_ctx.alarm_silence_remaining_s = silence_seconds[silence_idx];
        g_ctx.alarm_silence_tick_ms = 0U;
        set_message_text(g_ctx.alarm_silence_remaining_s ? "报警已静音" : "报警声音已恢复");
    }

    if(section == ECG_SECTION_ECG && item_index == 0U) {
        set_message_text("心电导联已切换为 %s", g_ecg_lead_opts[get_menu_value(ECG_SECTION_ECG, 0)]);
    }

    if(section == ECG_SECTION_PATIENT && item_index == 0U) {
        set_message_text("病人类型 %s", g_patient_profile_opts[get_menu_value(ECG_SECTION_PATIENT, 0)]);
    }

    if(section == ECG_SECTION_TEST) {
        if(item_index == 0U) {
            g_ctx.freeze_active = (get_menu_value(ECG_SECTION_TEST, 0) == 0);
            set_message_text(get_menu_value(ECG_SECTION_TEST, 0) ? "模拟波形已开启" : "模拟波形已关闭");
        }
        else if(item_index == 1U) {
            g_ctx.test_wave_fast = (get_menu_value(ECG_SECTION_TEST, 1) != 0);
            set_message_text(g_ctx.test_wave_fast ? "波形加速测试开启" : "波形加速测试关闭");
        }
        else if(item_index == 2U) {
            g_ctx.test_wave_pause = (get_menu_value(ECG_SECTION_TEST, 2) != 0);
            set_message_text(g_ctx.test_wave_pause ? "波形暂停测试开启" : "波形暂停测试关闭");
        }
        else if(item_index == 3U) {
            g_ctx.test_asystole = (get_menu_value(ECG_SECTION_TEST, 3) != 0);
            set_message_text(g_ctx.test_asystole ? "心脏停跳测试开启" : "心脏停跳测试关闭");
        }
    }

    refresh_ui();
}

static void adjust_focused_value(int delta)
{
    const ecg_menu_item_def_t * def = get_menu_item_def(g_ctx.active_section, g_ctx.focus_index);
    int value = get_menu_value(g_ctx.active_section, g_ctx.focus_index);

    if(def->kind == ECG_ITEM_ENUM && def->option_count > 0U) {
        int next = value + delta;

        if(next < 0) {
            next = def->option_count - 1;
        }
        else if(next >= def->option_count) {
            next = 0;
        }

        set_menu_value(g_ctx.active_section, g_ctx.focus_index, next);
        apply_live_setting_effects(g_ctx.active_section, g_ctx.focus_index);
        return;
    }

    if(def->kind == ECG_ITEM_INT) {
        int next = value + (delta * def->step);
        next = LV_MAX(def->min_value, LV_MIN(def->max_value, next));

        if(next != value) {
            set_menu_value(g_ctx.active_section, g_ctx.focus_index, next);
            apply_live_setting_effects(g_ctx.active_section, g_ctx.focus_index);
        }
    }
}

static void toggle_focused_enum_value(void)
{
    const ecg_menu_item_def_t * def = get_menu_item_def(g_ctx.active_section, g_ctx.focus_index);
    int value = get_menu_value(g_ctx.active_section, g_ctx.focus_index);
    int next;

    if(def->kind != ECG_ITEM_ENUM || def->option_count == 0U) {
        return;
    }

    next = value + 1;
    if(next >= def->option_count) {
        next = 0;
    }

    set_menu_value(g_ctx.active_section, g_ctx.focus_index, next);
    apply_live_setting_effects(g_ctx.active_section, g_ctx.focus_index);
}

static void execute_action_item(ecg_section_t section, uint8_t item_index)
{
    if(section == ECG_SECTION_NIBP && item_index == 0U) {
        start_nibp_cycle(!get_menu_value(ECG_SECTION_NIBP, 0));
        return;
    }

    if(section == ECG_SECTION_TEST && item_index == 4U) {
        g_prompt_tone_request_pending = true;
        set_message_text("提示音测试已触发");
        refresh_ui();
    }
}

static void update_nibp_status_label(void)
{
    if(g_ctx.nibp_time_label == NULL) {
        return;
    }

    if(get_menu_value(ECG_SECTION_NIBP, 0)) {
        if(g_ctx.nibp_cycle_elapsed_ms < 2000U) {
            lv_label_set_text(g_ctx.nibp_time_label, "充气中...");
            lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0xffaa44), 0);
            lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
        }
        else if(g_ctx.nibp_cycle_elapsed_ms < 5000U) {
            lv_label_set_text(g_ctx.nibp_time_label, "测量中...");
            lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0xffaa44), 0);
            lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
        }
        else {
            lv_label_set_text(g_ctx.nibp_time_label, "结果已就绪");
            lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0xffdd77), 0);
            lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
        }
    }
    else if(g_ctx.nibp_last_measure_time != 0) {
        struct tm * local_tm = localtime(&g_ctx.nibp_last_measure_time);

        if(local_tm) {
            set_label_text(g_ctx.nibp_time_label, "最近 %02d:%02d", local_tm->tm_hour, local_tm->tm_min);
        }
        else {
            lv_label_set_text(g_ctx.nibp_time_label, "最近 --:--");
        }
        lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0x5a3a08), 0);
        lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
    }
    else {
        lv_label_set_text(g_ctx.nibp_time_label, "空闲");
        lv_obj_set_style_text_color(g_ctx.nibp_time_label, lv_color_hex(0x5a3a08), 0);
        lv_obj_set_style_text_font(g_ctx.nibp_time_label, ECG_FONT_XS, 0);
    }
}

static void refresh_info_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    float t = (float)g_ctx.elapsed_ms / 1000.0f;
    int hr = 72 + (int)ecg_round_to_i32(sinf(t * 0.45f) * 4.0f);
    int spo2 = 98 + (int)ecg_round_to_i32(sinf(t * 0.28f) * 1.0f);
    int pr = (get_menu_value(ECG_SECTION_SPO2, 3) == 0) ? (hr - 1) : hr;
    float pi = 2.8f + sinf(t * 0.52f) * 0.4f;
    int sys = 118 + (int)ecg_round_to_i32(sinf(t * 0.16f) * 6.0f);
    int dia = 76 + (int)ecg_round_to_i32(cosf(t * 0.18f) * 4.0f);
    int map = dia + (sys - dia) / 3;

    if(g_ctx.test_asystole) {
        hr = 0;
        spo2 = 0;
        pr = 0;
        pi = 0.0f;
        sys = 0;
        dia = 0;
        map = 0;
    }

    if(g_ctx.pulse_dot) {
        g_ctx.pulse_on = !g_ctx.pulse_on;
        lv_obj_set_style_bg_opa(g_ctx.pulse_dot,
                                (g_ctx.test_asystole || !g_ctx.pulse_on) ? LV_OPA_20 : LV_OPA_COVER,
                                0);
    }

    update_clock_label();
    if(g_ctx.hr_value) set_label_text(g_ctx.hr_value, "%d", hr);
    if(g_ctx.spo2_value) set_label_text(g_ctx.spo2_value, "%d", spo2);
    if(g_ctx.pr_value) set_label_text(g_ctx.pr_value, "%d", pr);
    if(g_ctx.pi_value) set_label_text(g_ctx.pi_value, "%.1f", pi);
    if(g_ctx.nibp_sys_value) set_label_text(g_ctx.nibp_sys_value, "%d", sys);
    if(g_ctx.nibp_dia_value) set_label_text(g_ctx.nibp_dia_value, "%d", dia);
    if(g_ctx.nibp_map_value) set_label_text(g_ctx.nibp_map_value, "%d", map);

    if(g_ctx.test_asystole) {
        set_message_text("测试: 心脏停跳报警");
    }
    else if(hr >= get_menu_value(ECG_SECTION_ALARM, 2) - 5) {
        set_message_text("提示: 心率接近报警上限");
    }
    else if(spo2 <= get_menu_value(ECG_SECTION_ALARM, 3) + 1) {
        set_message_text("提示: 血氧接近报警下限");
    }

    if(get_menu_value(ECG_SECTION_NIBP, 0)) {
        g_ctx.nibp_cycle_elapsed_ms += ECG_INFO_PERIOD_MS;
        if(g_ctx.nibp_cycle_elapsed_ms >= 8000U) {
            start_nibp_cycle(false);
        }
    }

    if(g_ctx.alarm_silence_remaining_s > 0U) {
        g_ctx.alarm_silence_tick_ms += ECG_INFO_PERIOD_MS;

        while(g_ctx.alarm_silence_remaining_s > 0U && g_ctx.alarm_silence_tick_ms >= 1000U) {
            g_ctx.alarm_silence_tick_ms -= 1000U;
            g_ctx.alarm_silence_remaining_s--;
            if(g_ctx.alarm_silence_remaining_s == 0U) {
                set_menu_value(ECG_SECTION_ALARM, 0, 0);
                set_message_text("报警声音已恢复");
                g_ctx.alarm_silence_tick_ms = 0U;
            }
        }
    }
    else {
        g_ctx.alarm_silence_tick_ms = 0U;
    }

    update_nibp_status_label();
    refresh_ui();
}

static void push_wave_samples_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    float speed_mul = g_ctx.test_wave_fast ? 2.0f : 1.0f;
    float seconds = ((float)g_ctx.elapsed_ms / 1000.0f) * speed_mul;

    if((g_ctx.test_asystole || !g_ctx.freeze_active) && !g_ctx.test_wave_pause) {
        for(uint8_t i = 0; i < ECG_WAVE_ROW_COUNT; i++) {
            int32_t sample;

            if(g_ctx.waves[i].chart == NULL || g_ctx.waves[i].series == NULL) {
                continue;
            }

            if(g_ctx.test_asystole) {
                sample = 0;
            }
            else if(i == ECG_WAVE_ROW_COUNT - 1U) {
                sample = (int32_t)(spo2_wave_sample(seconds) * 95.0f);
            }
            else {
                sample = (int32_t)(ecg_wave_sample(seconds, i) * 110.0f);
            }

            lv_chart_set_next_value(g_ctx.waves[i].chart, g_ctx.waves[i].series, sample);
        }
    }

    if(!g_ctx.test_wave_pause) {
        g_ctx.elapsed_ms += ECG_SAMPLE_PERIOD_MS;
    }
}

static void cleanup_ctx_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    if(g_ctx.sample_timer) {
        lv_timer_del(g_ctx.sample_timer);
        g_ctx.sample_timer = NULL;
    }

    if(g_ctx.info_timer) {
        lv_timer_del(g_ctx.info_timer);
        g_ctx.info_timer = NULL;
    }

    memset(&g_ctx, 0, sizeof(g_ctx));
}

void ecg_monitor_keypad_apply_rotation(void)
{
    ECG_DISP_T * disp = ECG_DISP_GET_DEFAULT();

    if(disp) {
        int32_t hor_res;
        int32_t ver_res;

#if LVGL_VERSION_MAJOR >= 9
        hor_res = lv_display_get_horizontal_resolution(disp);
        ver_res = lv_display_get_vertical_resolution(disp);
#else
        hor_res = lv_disp_get_hor_res(disp);
        ver_res = lv_disp_get_ver_res(disp);
#endif

        if(ver_res > hor_res) {
            ECG_DISP_SET_ROTATION(disp, ECG_ROTATION_90);
        }
    }
}

lv_obj_t * ecg_monitor_keypad_create(lv_obj_t * parent)
{
    lv_obj_t * root;
    lv_obj_t * topbar;
    lv_obj_t * top_left;
    lv_obj_t * top_right;
    lv_obj_t * main;
    lv_obj_t * waves;
    lv_obj_t * params;
    lv_obj_t * dock;
    lv_obj_t * dock_row;
    lv_obj_t * dock_content;
    lv_obj_t * left_zone;
    lv_obj_t * right_zone;
    lv_obj_t * menu_list;

    memset(&g_ctx, 0, sizeof(g_ctx));

    g_ctx.active_section = ECG_SECTION_PATIENT;
    g_ctx.focus_index = 0;
    g_ctx.settings[ECG_SECTION_PATIENT][0] = 0;
    g_ctx.settings[ECG_SECTION_PATIENT][1] = 3;
    g_ctx.settings[ECG_SECTION_PATIENT][2] = 0;
    g_ctx.settings[ECG_SECTION_PATIENT][3] = 0;
    g_ctx.settings[ECG_SECTION_ECG][0] = 1;
    g_ctx.settings[ECG_SECTION_ECG][1] = 1;
    g_ctx.settings[ECG_SECTION_ECG][2] = 1;
    g_ctx.settings[ECG_SECTION_ECG][3] = 1;
    g_ctx.settings[ECG_SECTION_NIBP][0] = 0;
    g_ctx.settings[ECG_SECTION_NIBP][1] = 0;
    g_ctx.settings[ECG_SECTION_NIBP][2] = 1;
    g_ctx.settings[ECG_SECTION_NIBP][3] = 0;
    g_ctx.settings[ECG_SECTION_SPO2][0] = 1;
    g_ctx.settings[ECG_SECTION_SPO2][1] = 1;
    g_ctx.settings[ECG_SECTION_SPO2][2] = 0;
    g_ctx.settings[ECG_SECTION_SPO2][3] = 0;
    g_ctx.settings[ECG_SECTION_ALARM][0] = 0;
    g_ctx.settings[ECG_SECTION_ALARM][1] = 3;
    g_ctx.settings[ECG_SECTION_ALARM][2] = 120;
    g_ctx.settings[ECG_SECTION_ALARM][3] = 90;
    g_preview_enable_shortcut_refresh = false;

    root = lv_obj_create(parent);
    g_ctx.screen = root;

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(root, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(root, lv_color_hex(ECG_TEXT_SECONDARY), 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    topbar = lv_obj_create(root);
    lv_obj_remove_style_all(topbar);
    lv_obj_set_size(topbar, LV_PCT(100), ECG_TOPBAR_H);
    lv_obj_align(topbar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(topbar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(topbar, 1, 0);
    lv_obj_set_style_border_color(topbar, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_style_pad_left(topbar, ECG_TOPBAR_PAD_X, 0);
    lv_obj_set_style_pad_right(topbar, ECG_TOPBAR_PAD_X, 0);
    lv_obj_set_layout(topbar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    top_left = lv_obj_create(topbar);
    lv_obj_remove_style_all(top_left);
    lv_obj_set_height(top_left, LV_SIZE_CONTENT);
    lv_obj_set_width(top_left, LV_SIZE_CONTENT);
    lv_obj_set_layout(top_left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(top_left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(top_left, ECG_TOPBAR_GAP, 0);

    lv_obj_t * product = lv_label_create(top_left);
    lv_label_set_text(product, "ECG-5L PortaMon");
    lv_obj_set_style_text_font(product, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(product, lv_color_hex(ECG_TEXT_TERTIARY), 0);

    g_ctx.patient_badge = lv_label_create(top_left);
    lv_obj_set_style_text_font(g_ctx.patient_badge, ECG_FONT_XS, 0);
    lv_obj_set_style_text_color(g_ctx.patient_badge, lv_color_hex(0xe0db8a), 0);
    lv_obj_set_style_bg_color(g_ctx.patient_badge, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_bg_opa(g_ctx.patient_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_ctx.patient_badge, 1, 0);
    lv_obj_set_style_border_color(g_ctx.patient_badge, lv_color_hex(ECG_BORDER_SOFT), 0);
    lv_obj_set_style_radius(g_ctx.patient_badge, 4, 0);
    lv_obj_set_style_pad_left(g_ctx.patient_badge, ECG_BADGE_PAD_X, 0);
    lv_obj_set_style_pad_right(g_ctx.patient_badge, ECG_BADGE_PAD_X, 0);
    lv_obj_set_style_pad_top(g_ctx.patient_badge, ECG_BADGE_PAD_Y, 0);
    lv_obj_set_style_pad_bottom(g_ctx.patient_badge, ECG_BADGE_PAD_Y, 0);

    g_ctx.pulse_dot = lv_obj_create(top_left);
    lv_obj_remove_style_all(g_ctx.pulse_dot);
    lv_obj_set_size(g_ctx.pulse_dot, ECG_PULSE_DOT_SIZE, ECG_PULSE_DOT_SIZE);
    lv_obj_set_style_radius(g_ctx.pulse_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(g_ctx.pulse_dot, lv_color_hex(0x00ff88), 0);

    g_ctx.message_label = lv_label_create(top_left);
    lv_obj_set_style_text_font(g_ctx.message_label, ECG_FONT_XS, 0);
    lv_obj_set_style_text_color(g_ctx.message_label, lv_color_hex(ECG_TEXT_SECONDARY), 0);

    top_right = lv_obj_create(topbar);
    lv_obj_remove_style_all(top_right);
    lv_obj_set_height(top_right, LV_SIZE_CONTENT);
    lv_obj_set_width(top_right, LV_SIZE_CONTENT);
    lv_obj_set_layout(top_right, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(top_right, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(top_right, ECG_TOPBAR_GAP, 0);

    g_ctx.alarm_badge = lv_label_create(top_right);
    lv_obj_set_style_text_font(g_ctx.alarm_badge, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_bg_color(g_ctx.alarm_badge, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_bg_opa(g_ctx.alarm_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_ctx.alarm_badge, 1, 0);
    lv_obj_set_style_border_color(g_ctx.alarm_badge, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_style_radius(g_ctx.alarm_badge, 4, 0);
    lv_obj_set_style_pad_left(g_ctx.alarm_badge, ECG_BADGE_PAD_X, 0);
    lv_obj_set_style_pad_right(g_ctx.alarm_badge, ECG_BADGE_PAD_X, 0);
    lv_obj_set_style_pad_top(g_ctx.alarm_badge, ECG_BADGE_PAD_Y, 0);
    lv_obj_set_style_pad_bottom(g_ctx.alarm_badge, ECG_BADGE_PAD_Y, 0);

    g_ctx.clock_label = lv_label_create(top_right);
    lv_label_set_text(g_ctx.clock_label, "00:00:00");
    lv_obj_set_style_text_font(g_ctx.clock_label, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(g_ctx.clock_label, lv_color_hex(ECG_TEXT_TERTIARY), 0);

    lv_obj_t * battery = lv_label_create(top_right);
    lv_label_set_text(battery, "|||| 84%");
    lv_obj_set_style_text_font(battery, ECG_FONT_EN_XS, 0);
    lv_obj_set_style_text_color(battery, lv_color_hex(ECG_TEXT_TERTIARY), 0);

    main = lv_obj_create(root);
    lv_obj_remove_style_all(main);
    lv_obj_set_size(main, LV_PCT(100), ECG_LOGICAL_H - ECG_TOPBAR_H - ECG_DOCK_H);
    lv_obj_align(main, LV_ALIGN_TOP_MID, 0, ECG_TOPBAR_H);
    lv_obj_set_style_bg_color(main, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(main, LV_OPA_COVER, 0);
    lv_obj_set_layout(main, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    waves = lv_obj_create(main);
    lv_obj_remove_style_all(waves);
    lv_obj_set_width(waves, ECG_LOGICAL_W - ECG_PARAM_PANEL_W);
    lv_obj_set_height(waves, LV_PCT(100));
    lv_obj_set_style_bg_color(waves, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(waves, LV_OPA_COVER, 0);
    lv_obj_set_layout(waves, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(waves, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(waves, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    create_wave_rows(waves);

    g_ctx.menu_panel = lv_obj_create(waves);
    lv_obj_remove_style_all(g_ctx.menu_panel);
    lv_obj_set_size(g_ctx.menu_panel, ECG_MENU_PANEL_W, ECG_MENU_PANEL_H);
    lv_obj_align(g_ctx.menu_panel, LV_ALIGN_BOTTOM_LEFT, ECG_MENU_PANEL_MARGIN, -ECG_MENU_PANEL_MARGIN);
    lv_obj_set_style_bg_color(g_ctx.menu_panel, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
    lv_obj_set_style_bg_opa(g_ctx.menu_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_ctx.menu_panel, 12, 0);
    lv_obj_set_style_border_width(g_ctx.menu_panel, 1, 0);
    lv_obj_set_style_pad_all(g_ctx.menu_panel, 18, 0);
    lv_obj_set_layout(g_ctx.menu_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(g_ctx.menu_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_ctx.menu_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(g_ctx.menu_panel, LV_OBJ_FLAG_FLOATING);

    g_ctx.menu_title = lv_label_create(g_ctx.menu_panel);
    lv_obj_set_style_text_font(g_ctx.menu_title, ECG_FONT_LG, 0);

    menu_list = lv_obj_create(g_ctx.menu_panel);
    lv_obj_remove_style_all(menu_list);
    lv_obj_set_width(menu_list, LV_PCT(100));
    lv_obj_set_flex_grow(menu_list, 1);
    lv_obj_set_layout(menu_list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(menu_list, 8, 0);

    for(uint8_t i = 0; i < ECG_MENU_ITEM_COUNT; i++) {
        lv_obj_t * row = lv_obj_create(menu_list);
        lv_obj_t * item_label = lv_label_create(row);
        lv_obj_t * value_label = lv_label_create(row);

        g_ctx.menu_rows[i] = row;
        g_ctx.menu_item_labels[i] = item_label;
        g_ctx.menu_value_labels[i] = value_label;

        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_height(row, ECG_MENU_ROW_H);
        lv_obj_set_style_bg_color(row, lv_color_hex(ECG_BG_SURFACE_ALT), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_set_style_pad_left(row, 12, 0);
        lv_obj_set_style_pad_right(row, 12, 0);
        lv_obj_set_style_pad_top(row, 10, 0);
        lv_obj_set_style_pad_bottom(row, 10, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_set_style_text_font(item_label, ECG_FONT_MD, 0);
        lv_obj_set_style_text_font(value_label, ECG_FONT_EN_SM, 0);
    }

    g_ctx.menu_hint = lv_label_create(g_ctx.menu_panel);
    lv_obj_set_style_text_font(g_ctx.menu_hint, ECG_FONT_XS, 0);
    lv_obj_set_style_text_color(g_ctx.menu_hint, lv_color_hex(ECG_TEXT_TERTIARY), 0);

    g_ctx.freeze_overlay = lv_label_create(waves);
    lv_label_set_text(g_ctx.freeze_overlay, "FREEZE");
    lv_obj_set_style_text_font(g_ctx.freeze_overlay, ECG_FONT_XL, 0);
    lv_obj_set_style_text_color(g_ctx.freeze_overlay, lv_color_hex(0xcde5cd), 0);
    lv_obj_set_style_bg_color(g_ctx.freeze_overlay, lv_color_hex(ECG_BG_OVERLAY), 0);
    lv_obj_set_style_bg_opa(g_ctx.freeze_overlay, LV_OPA_60, 0);
    lv_obj_set_style_pad_left(g_ctx.freeze_overlay, 18, 0);
    lv_obj_set_style_pad_right(g_ctx.freeze_overlay, 18, 0);
    lv_obj_set_style_pad_top(g_ctx.freeze_overlay, 8, 0);
    lv_obj_set_style_pad_bottom(g_ctx.freeze_overlay, 8, 0);
    lv_obj_set_style_radius(g_ctx.freeze_overlay, 10, 0);
    lv_obj_align(g_ctx.freeze_overlay, LV_ALIGN_TOP_MID, 0, ECG_FREEZE_OVERLAY_Y);
    lv_obj_add_flag(g_ctx.freeze_overlay, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_HIDDEN);

    params = lv_obj_create(main);
    lv_obj_remove_style_all(params);
    lv_obj_set_width(params, ECG_PARAM_PANEL_W);
    lv_obj_set_height(params, LV_PCT(100));
    lv_obj_set_style_bg_color(params, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(params, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(params, LV_BORDER_SIDE_LEFT, 0);
    lv_obj_set_style_border_width(params, 1, 0);
    lv_obj_set_style_border_color(params, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_layout(params, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(params, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(params, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    create_hr_block(params);
    create_nibp_block(params);
    create_spo2_block(params);
    create_pi_block(params);

    dock = lv_obj_create(root);
    lv_obj_remove_style_all(dock);
    lv_obj_set_size(dock, LV_PCT(100), ECG_DOCK_H);
    lv_obj_align(dock, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(dock, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(dock, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(dock, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(dock, 1, 0);
    lv_obj_set_style_border_color(dock, lv_color_hex(ECG_BORDER_SUBTLE), 0);
    lv_obj_set_style_pad_left(dock,
#ifdef ECG_SDL_PREVIEW
                              16
#else
                              24
#endif
    , 0);
    lv_obj_set_style_pad_right(dock,
#ifdef ECG_SDL_PREVIEW
                               16
#else
                               24
#endif
    , 0);
    lv_obj_set_style_pad_top(dock,
#ifdef ECG_SDL_PREVIEW
                             8
#else
                             6
#endif
    , 0);
    lv_obj_set_style_pad_bottom(dock,
#ifdef ECG_SDL_PREVIEW
                                6
#else
                                6
#endif
    , 0);

    dock_row = lv_obj_create(dock);
    lv_obj_remove_style_all(dock_row);
    lv_obj_set_size(dock_row, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(dock_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dock_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dock_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(dock_row,
#ifdef ECG_SDL_PREVIEW
                                8
#else
                                10
#endif
    , 0);

    dock_content = dock_row;

    left_zone = create_dock_zone_panel(dock_content,
#ifdef ECG_SDL_PREVIEW
                                       260
#else
                                       380
#endif
    );
    create_compact_control_zone(left_zone);

    right_zone = create_dock_zone_panel(dock_content, 0);
    lv_obj_set_flex_grow(right_zone, 1);
    create_function_zone_compact(right_zone);

    update_clock_label();
    set_message_text("");
    refresh_ui();
    update_nibp_status_label();

    g_ctx.sample_timer = lv_timer_create(push_wave_samples_cb, ECG_SAMPLE_PERIOD_MS, NULL);
    g_ctx.info_timer = lv_timer_create(refresh_info_cb, ECG_INFO_PERIOD_MS, NULL);

    lv_obj_add_event_cb(root, cleanup_ctx_cb, LV_EVENT_DELETE, NULL);

    return root;
}

void ecg_monitor_keypad_load(void)
{
    lv_obj_t * screen;

    ecg_monitor_keypad_apply_rotation();

    screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(ECG_BG_ROOT), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    ecg_monitor_keypad_create(screen);
    ECG_SCREEN_LOAD(screen);
}

void ecg_monitor_keypad_handle_input(ecg_monitor_input_action_t action)
{
    if(g_ctx.screen == NULL) {
        if(ECG_SCREEN_ACTIVE()) {
            ecg_monitor_keypad_create(ECG_SCREEN_ACTIVE());
        }
        else {
            return;
        }
    }

    switch(action) {
        case ECG_INPUT_OK: {
            const ecg_menu_item_def_t * def;

            if(!g_ctx.menu_open) {
                open_section(g_ctx.active_section);
                break;
            }

            def = get_menu_item_def(g_ctx.active_section, g_ctx.focus_index);
            if(def->kind == ECG_ITEM_ACTION) {
                execute_action_item(g_ctx.active_section, g_ctx.focus_index);
            }
            else if(g_ctx.active_section == ECG_SECTION_TEST && def->kind == ECG_ITEM_ENUM) {
                toggle_focused_enum_value();
            }
            else {
                g_ctx.edit_mode = !g_ctx.edit_mode;
                set_message_text(g_ctx.edit_mode ? "正在编辑 %s" : "%s 已保存", def->label);
                refresh_ui();
            }
            break;
        }

        case ECG_INPUT_BACK:
            if(g_ctx.edit_mode) {
                g_ctx.edit_mode = false;
                set_message_text("已退出编辑");
            }
            else if(g_ctx.menu_open) {
                g_ctx.menu_open = false;
                set_message_text("菜单已关闭");
            }
            else {
                set_message_text("就绪");
            }
            refresh_ui();
            break;

        case ECG_INPUT_KNOB_CW:
            if(!g_ctx.menu_open) {
                open_section(g_ctx.active_section);
            }
            else if(g_ctx.edit_mode) {
                adjust_focused_value(1);
            }
            else {
                g_ctx.focus_index = (g_ctx.focus_index + 1U) % get_menu_item_count(g_ctx.active_section);
                refresh_ui();
            }
            break;

        case ECG_INPUT_KNOB_CCW:
            if(!g_ctx.menu_open) {
                open_section(g_ctx.active_section);
            }
            else if(g_ctx.edit_mode) {
                adjust_focused_value(-1);
            }
            else {
                g_ctx.focus_index = (g_ctx.focus_index == 0U) ? (get_menu_item_count(g_ctx.active_section) - 1U) : (g_ctx.focus_index - 1U);
                refresh_ui();
            }
            break;

        case ECG_INPUT_PATIENT:
            open_section(ECG_SECTION_PATIENT);
            break;

        case ECG_INPUT_ECG:
            open_section(ECG_SECTION_ECG);
            break;

        case ECG_INPUT_NIBP:
            open_section(ECG_SECTION_NIBP);
            break;

        case ECG_INPUT_SPO2:
            open_section(ECG_SECTION_SPO2);
            break;

        case ECG_INPUT_ALARM:
            open_section(ECG_SECTION_ALARM);
            break;

        case ECG_INPUT_FREEZE:
            g_ctx.freeze_active = !g_ctx.freeze_active;
            set_message_text(g_ctx.freeze_active ? "波形已冻结" : "波形已恢复");
            refresh_ui();
            break;
    }
}
