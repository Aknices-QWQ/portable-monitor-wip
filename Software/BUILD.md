# Build Options

## 1. Local SDL2 preview

This uses the real LVGL monitor UI from `src/app/ecg_monitor_keypad_lvgl.c`.

Initialize third-party dependencies after cloning:

```bash
git submodule update --init --recursive
```

Build:

```bash
./build_sdl2_preview.sh
```

Run:

```bash
./build/sdl2/ecg_monitor_sdl2_preview
```

SDL keyboard simulation:

- `Enter`: Left1 knob press / OK
- `Esc` or `Backspace`: Left2 / Back
- `Left` or `[`: knob CCW
- `Right` or `]`: knob CW
- `1`: Left3 / PATIENT
- `2`: Left4 / ECG
- `3`: Left5 / NIBP
- `4`: Left6 / SpO2
- `5`: Left7 / ALARM
- `6`: Left8 / FREEZE

Notes:

- Requires `cmake`, `python3`, `pkg-config`, and `libsdl2-dev`.
- This path runs in landscape on the PC and uses the same menu logic as the device build.

## 2. Cross-compile for T113 device

This uses the SDK in `talowe-T113-I-Tina-sdk` and builds a framebuffer target for the real panel.

Build:

```bash
./build_device.sh
```

Output:

```bash
./Software/ecg_monitor_t113
```

Board input mapping used by the build:

- `Left1`: rotary push on `R2C1`
- `Left2`: `R2C2`
- `Left3`: `R2C3`
- `Left4`: `R2C4`
- `Left5`: `R1C4`
- `Left6`: `R1C3`
- `Left7`: `R1C2`
- `Left8`: `R1C1`
- matrix scan pins:
  `COL4=PD15`, `COL3=PD16`, `COL2=PD17`, `COL1=PD18`, `ROW2=PD20`, `ROW1=PD21`
- rotary A/B:
  `PG16=A`, `PG13=B`

Notes:

- Default compiler:
  `/home/talowe/Desktop/T113/talowe-T113-I-Tina-sdk/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain/bin/arm-openwrt-linux-gnueabi-gcc`
- Override it if needed:

```bash
make -f Makefile.device CC=/path/to/your/arm-openwrt-linux-gnueabi-gcc
```

- The device binary scans the matrix keypad and encoder directly through `/sys/class/gpio`.
- The rotary decoder currently assumes `4` quadrature transitions per detent. If your specific encoder feels too slow or too fast, adjust `ENCODER_TRANSITIONS_PER_DETENT` in `src/platform/device_main.c`.
