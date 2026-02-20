# HiTerm v1.1.0 - Chinese Support / 中文支持版

## New Features / 新功能

- **Bilingual UI** — Switch between English and Chinese in Settings
- **Chinese Font Rendering** — efontCN_16 font for displaying Chinese characters
- **Pinyin Input Method** — Type pinyin to input Chinese characters with candidate selection
- **双语界面** — 在设置中切换中英文
- **中文字体渲染** — efontCN_16 字体显示中文
- **拼音输入法** — 输入拼音选择候选字

## All Features / 所有功能

- **P2P Encrypted Chat** — Direct device-to-device messaging via ESP-NOW with end-to-end AES-128-GCM encryption
- **Device Pairing** — 6-digit pairing code system, supports up to 8 paired devices
- **Message History** — Stored on SD card, per-conversation
- **Range Test** — Real-time signal quality monitoring
- **Power Management** — Screen timeout (5/10/15/30m) + deep sleep (15/30/60m/Off)
- **Sound Notifications** — Configurable volume levels (Mute/Low/Med/High)
- **Relay Support** — Multi-hop message forwarding (up to 3 hops) to extend communication range

## Hardware Requirements / 硬件要求

- M5Cardputer (ESP32-S3, 240x135 LCD, 56-key keyboard)
- MicroSD card (required for Pinyin input, optional for message history)
- ESP32 board(s) for relay (optional, any ESP32 variant)

## Firmware Files / 固件文件

- **HiTerm-M5Cardputer-v1.1.0-CN.bin** — Main chat device firmware for M5Cardputer / M5Cardputer 主设备固件
- **HiTerm-Relay-ESP32C3-v1.1.0.bin** — Relay firmware for ESP32 / ESP32 中继固件
- **pinyin_dict.txt** — Pinyin dictionary (copy to SD card: `/chat/pinyin_dict.txt`) / 拼音词库（复制到 SD 卡：`/chat/pinyin_dict.txt`）

## Flashing Instructions / 刷写说明

### M5Cardputer

Using esptool.py:
```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-M5Cardputer-v1.1.0-CN.bin
```

Or use [M5Burner](https://docs.m5stack.com/en/download) for a GUI experience.

### Relay Device / 中继设备

```bash
esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-Relay-ESP32C3-v1.1.0.bin
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
