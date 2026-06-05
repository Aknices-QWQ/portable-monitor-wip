cmake_minimum_required(VERSION 3.16)
project(ecg_monitor_lvgl_targets C)

set(CMAKE_C_STANDARD 99)

set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/lv_conf.h" CACHE PATH "" FORCE)
set(LV_BUILD_SET_CONFIG_OPTS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LV_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_SDL ON CACHE BOOL "" FORCE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_subdirectory(third_party/lvgl-master)

add_executable(ecg_monitor_sdl2_preview
    src/platform/lvgl_sdl_main.c
    src/app/ecg_monitor_keypad_lvgl.c
    assets/fonts/generated_fonts/ecg_font_zh_16.c
    assets/fonts/generated_fonts/ecg_font_zh_20.c
    assets/fonts/generated_fonts/ecg_font_zh_28.c
    assets/fonts/generated_fonts/ecg_font_en_16.c
    assets/fonts/generated_fonts/ecg_font_en_20.c
    assets/fonts/generated_fonts/ecg_font_en_22.c
    assets/fonts/generated_fonts/ecg_font_en_36.c
    assets/fonts/generated_fonts/ecg_font_en_48.c
    assets/fonts/generated_fonts/ecg_font_en_56.c
    assets/fonts/generated_fonts/ecg_font_en_64.c
)

target_include_directories(ecg_monitor_sdl2_preview PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
)
target_compile_definitions(ecg_monitor_sdl2_preview PRIVATE ECG_SDL_PREVIEW=1)
target_link_libraries(ecg_monitor_sdl2_preview PRIVATE lvgl m ${SDL2_LIBRARIES})
