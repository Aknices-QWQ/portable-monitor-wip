#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*
 * Minimal LVGL configuration for the local SDL2 preview target.
 * Unspecified options fall back to LVGL's internal defaults.
 */

#define LV_COLOR_DEPTH 32

#define LV_USE_SDL 1

#define LV_USE_SYSMON 1
#if LV_USE_SYSMON
    #define LV_SYSMON_GET_IDLE lv_timer_get_idle
    #define LV_USE_PERF_MONITOR 1
    #if LV_USE_PERF_MONITOR
        #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
        #define LV_USE_PERF_MONITOR_LOG_MODE 0
    #endif
    #define LV_USE_MEM_MONITOR 1
    #if LV_USE_MEM_MONITOR
        #define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
    #endif
#endif

#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_14

#endif
