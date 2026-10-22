# OpenRGB-Onda

昂达（Onda）主板 RGB 的 OpenRGB 驱动 + Windows 云端构建。

## 这是什么

- **驱动**：`Controllers/OndaController/` —— 让 OpenRGB 支持昂达主板的 RGB 控制器
  （设备名 `snq.cn - Lighting Controller`，HID `VID 8888 / PID 7A95`，Usage Page 0xFF01）
- **协议来源**：对 `OndaRGB.exe` / `RainAPI.LED.dll` 的静态逆向（详见协议规范文档）
- **布局**：8 条灯带 × 60 LED = 480，Direct 逐 LED 模式
- **构建**：GitHub Actions 自动克隆上游 [OpenRGB](https://github.com/CalcProgrammer1/OpenRGB)，
  注入本驱动，用 Qt 6.8.3 + MSVC 2022 编译出 Windows 64 位成品

## 设备协议要点（Feature Report，hidapi）

| 报文 | 格式 |
|---|---|
| 握手 | `80 00 00 00 00 00 00 00` → 回读 8 字节，版本 = `(resp[5]<<8)\|resp[6]` |
| 直连 RGB | 每灯带 256 字节：`[0x10+条带号, 0x00/0xFF(末条), 条带号, 60×RGB, 零填充]`，间隔 1ms |

## 获取构建成品

1. 本仓库 **Actions** → 最近一次 workflow run → Artifacts → 下载 `OpenRGB Windows 64-bit`
2. 解压后直接运行 `OpenRGB.exe`（**先退出昂达官方 OndaRGB.exe**，避免互相覆盖灯效）

## 本地构建

```powershell
git clone --depth 1 https://github.com/CalcProgrammer1/OpenRGB.git
Copy-Item -Recurse -Force Controllers/OndaController OpenRGB/Controllers/
cd OpenRGB
qmake OpenRGB.pro CONFIG-=debug_and_release CONFIG+=release
jom
windeployqt .\release\
```

需要：Qt 6.8.3 (msvc2022_64) + Visual Studio 2022 BuildTools + jom。

## 已知限制

- 仅支持 Direct 逐 LED 模式；内置效果（呼吸/渐变等）的载荷格式尚未逆向，未实现
- PID 0x7AE0 变体已在代码中注册，但未在真机验证
- 协议结论来自静态逆向，未经抓包验证（验证脚本见原分析工作区）

## 许可

- 驱动代码：GPL-2.0-or-later（与 OpenRGB 一致，模板源自 OpenRGB NollieController）
- 协议逆向结论：仅供互操作性开发使用
