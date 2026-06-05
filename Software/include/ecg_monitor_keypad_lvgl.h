#ifndef ECG_MONITOR_KEYPAD_LVGL_H
#define ECG_MONITOR_KEYPAD_LVGL_H

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ECG_INPUT_OK = 0,
    ECG_INPUT_BACK,
    ECG_INPUT_KNOB_CW,
    ECG_INPUT_KNOB_CCW,
    ECG_INPUT_PATIENT,
    ECG_INPUT_ECG,
    ECG_INPUT_NIBP,
    ECG_INPUT_SPO2,
    ECG_INPUT_ALARM,
    ECG_INPUT_FREEZE
} ecg_monitor_input_action_t;

/*
 * Loads the ECG monitor screen onto the default display.
 * The UI is built for a 1920x1280 logical canvas and the display
 * is rotated 90 degrees clockwise so it fits a 1280x1920 panel.
 */
void ecg_monitor_keypad_load(void);

/*
 * Creates the ECG monitor content under an existing parent.
 * Call ecg_monitor_keypad_apply_rotation() first if the panel is 1280x1920.
 */
lv_obj_t * ecg_monitor_keypad_create(lv_obj_t * parent);

/*
 * Applies a 90 degree clockwise rotation to the default display.
 */
void ecg_monitor_keypad_apply_rotation(void);

/*
 * Feeds a non-touch input action into the UI.
 * This is used both by the device-side GPIO/encoder scanner and the SDL2 preview.
 */
void ecg_monitor_keypad_handle_input(ecg_monitor_input_action_t action);

/*
 * Opens the hidden SDL test menu.
 */
void ecg_monitor_keypad_open_test_menu(void);

/*
 * Returns true when the UI is currently in a high-priority alarm state.
 */
bool ecg_monitor_keypad_is_high_priority_alarm_active(void);

/*
 * Returns and clears a pending one-shot prompt tone test request.
 * SDL preview uses this to play a local试听音，不影响设备侧逻辑。
 */
bool ecg_monitor_keypad_take_prompt_tone_request(void);

#ifdef __cplusplus
}
#endif

#endif
