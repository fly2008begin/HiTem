# HiTerm v1.0.0 - English Version

## Features

- **P2P Encrypted Chat** — Direct device-to-device messaging via ESP-NOW with end-to-end AES-128-GCM encryption
- **Device Pairing** — 6-digit pairing code system, supports up to 8 paired devices
- **Message History** — Stored on SD card, per-conversation
- **Range Test** — Real-time signal quality monitoring
- **Power Management** — Screen timeout (5/10/15/30m) + deep sleep (15/30/60m/Off)
- **Sound Notifications** — Configurable volume levels (Mute/Low/Med/High)
- **Relay Support** — Multi-hop message forwarding (up to 3 hops) to extend communication range

## Hardware Requirements

- M5Cardputer (ESP32-S3, 240x135 LCD, 56-key keyboard)
- MicroSD card (optional, for message history)
- ESP32 board(s) for relay (optional, any ESP32 variant)

## Firmware Files

- **HiTerm-M5Cardputer-v1.0.0-EN.bin** — Main chat device firmware for M5Cardputer
- **HiTerm-Relay-ESP32C3-v1.0.0.bin** — Relay firmware for ESP32 (tested on ESP32-C3, compatible with all ESP32 variants)

## Flashing Instructions

### M5Cardputer

Using esptool.py:
```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-M5Cardputer-v1.0.0-EN.bin
```

Or use [M5Burner](https://docs.m5stack.com/en/download) for a GUI experience.

### Relay Device

```bash
esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x0 HiTerm-Relay-ESP32C3-v1.0.0.bin
```

For other ESP32 variants, change `--chip` accordingly (esp32, esp32s2, esp32s3, esp32c3).

## Quick Start

1. Flash firmware to M5Cardputer
2. Power on, navigate to **Pair** menu
3. Both devices will auto-discover and exchange keys
4. Select **Chat** to start messaging

## Documentation

See [README.md](https://github.com/YOUR_USERNAME/hiTem/blob/main/README.md) for complete documentation.
