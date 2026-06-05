# Software

便携监护仪的软件原型目录，当前主要包含 LVGL 监护界面、SDL2 本地预览、T113 设备端入口、字体资源和相关资料。

## 当前状态

- UI：已有 LVGL 监护界面原型
- 本地预览：支持 SDL2 预览和键盘模拟输入
- 设备端：预留 T113 framebuffer 运行入口和 GPIO 按键/旋钮扫描
- 数据采集：仍需补充真实传感器驱动、协议和算法
- 医疗用途：未验证，不能用于诊断、治疗或正式监护
- 许可证：软件源码为 PolyForm Noncommercial 1.0.0，禁止未经许可的商业使用

## 目录结构

```text
Software/
├── src/
│   ├── app/                 # LVGL 应用界面
│   └── platform/            # SDL2 预览和设备端入口
├── include/                 # 公共头文件和 LVGL 配置
├── assets/
│   ├── fonts/               # 字体源文件与生成字体
│   ├── previews/            # UI 预览图
│   └── prototype/           # 早期原型/参考代码
├── docs/
│   └── datasheets/          # 芯片、屏幕等资料
├── third_party/
│   └── lvgl-master/         # LVGL 第三方源码
├── BUILD.md                 # 构建说明
├── CMakeLists.txt           # SDL2 预览构建
├── Makefile.device          # T113 设备端构建
├── build_sdl2_preview.sh
└── build_device.sh
```

## 构建

详细命令见 [BUILD.md](BUILD.md)。

首次 clone 后请先初始化第三方依赖：

```bash
git submodule update --init --recursive
```

本地 SDL2 预览：

```bash
./build_sdl2_preview.sh
./build/sdl2/ecg_monitor_sdl2_preview
```

T113 设备端：

```bash
./build_device.sh
```

## 贡献方向

欢迎提交以下贡献：

- LVGL 界面布局和交互优化
- SDL2 预览问题修复
- T113 设备端适配
- ECG / SpO2 / NIBP / TEMP 等真实数据驱动
- 串口、SPI、I2C、GPIO、显示和输入驱动
- 数据协议、日志和测试工具
- 构建脚本、文档和测试流程

## 提交注意

请不要提交：

- `build/`
- `node_modules/`
- `.o`、可执行文件和交叉编译产物
- 私钥、Token、密码、设备证书
- 未脱敏的真实健康数据

## 许可证

`Software/` 中的软件源码使用 PolyForm Noncommercial License 1.0.0。第三方依赖和带有独立许可证的文件保留其原许可证。
