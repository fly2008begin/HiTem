# HiTerm v1.1.1 - Chinese Support / 中文支持版

## Bug Fixes / 问题修复

- **Fixed Chinese display in English UI mode** — Messages now display Chinese characters correctly regardless of UI language setting
- **修复英文界面下中文显示问题** — 消息内容现在无论界面语言设置如何都能正确显示中文

Previously, when the UI language was set to English, received Chinese messages would display as garbled characters. Now message content always uses the Chinese-capable font (efontCN_16), which supports both English and Chinese characters, while UI elements (menus, titles) still respect the language setting.

之前当界面语言设置为英文时，接收到的中文消息会显示为乱码。现在消息内容始终使用支持中文的字体（efontCN_16），该字体同时支持英文和中文字符，而界面元素（菜单、标题）仍然遵循语言设置。

## All Features / 所有功能

- **P2P Encrypted Chat** — Direct device-to-device messaging via ESP-NOW with end-to-end AES-128-GCM encryption
- **Device Pairing** — 6-digit pairing code system, supports up to 8 paired devices
- **Message History** — Stored on SD card, per-conversation
- **Range Test** — Real-time signal quality monitoring
- **Power Management** — Screen timeout (5/10/15/30m) + deep sleep (15/30/60m/Off)
- **Sound Notifications** — Configurable volume levels (Mute/Low/Med/High)
- **Relay Support** — Multi-hop message forwarding (up to 3 hops) to extend communication range
- **Bilingual UI** — Switch between English and Chinese in Settings
- **Chinese Font Rendering** — efontCN_16 font for displaying Chinese characters
- **Pinyin Input Method** — Type pinyin to input Chinese characters with candidate selection

## Hardware Requirements / 硬件要求

- M5Cardputer (ESP32-S3, 240x135 LCD, 56-key keyboard)
- MicroSD card (required for Pinyin input, optional for message history)
- ESP32 board(s) for relay (optional, any ESP32 variant)

## Firmware Files / 固件文件

- **HiTerm-M5Cardputer-v1.1.1-CN.bin** — Main chat device firmware for M5Cardputer / M5Cardputer 主设备固件
- **HiTerm-Relay-ESP32C3-v1.1.1.bin** — Relay firmware for ESP32 / ESP32 中继固件
- **pinyin_dict.txt** — Pinyin dictionary (copy to SD card: `/chat/pinyin_dict.txt`) / 拼音词库（复制到 SD 卡：`/chat/pinyin_dict.txt`）

## Flashing Instructions / 刷写说明

### M5Cardputer

Using esptool.py:
```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-M5Cardputer-v1.1.1-CN.bin
```

Or use [M5Burner](https://docs.m5stack.com/en/download) for a GUI experience.

### Relay Device / 中继设备

```bash
esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-Relay-ESP32C3-v1.1.1.bin
```

For other ESP32 variants, change `--chip` accordingly (esp32, esp32s2, esp32s3, esp32c3).

### Pinyin Dictionary Setup / 拼音词库设置

Copy `pinyin_dict.txt` to your SD card:
```
SD card root/
└── chat/
    └── pinyin_dict.txt
```

Without this file, Pinyin input will be unavailable (English input still works).

## Using Pinyin Input / 使用拼音输入

1. In Chat, press `Fn` + `Space` to toggle CN/EN input mode
2. Type pinyin letters (e.g. `ni`), candidates appear above input
3. Press `1`-`5` to select a candidate, or `Space` for the first one
4. `;` / `.` to page Up/Dn through candidates

## Documentation / 文档

See [README.md](https://github.com/YOUR_USERNAME/hiTem/blob/chinese-support/README.md) for complete documentation.
