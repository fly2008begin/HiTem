# HiTerm — ESP-NOW P2P Encrypted Chat for M5Cardputer

A peer-to-peer encrypted chat tool built on M5Cardputer using ESP-NOW protocol. Features end-to-end AES-128-GCM encryption, ECDH key exchange, and a cyber-glow UI.

## Features

- **P2P Chat** — Direct device-to-device messaging via ESP-NOW
- **End-to-End Encryption** — ECDH key exchange + AES-128-GCM
- **Device Pairing** — 6-digit pairing code, up to 8 paired devices
- **Message History** — Stored on SD card, per-conversation
- **Range Test** — Real-time signal quality monitoring
- **Power Management** — Screen timeout + deep sleep
- **Sound Notifications** — Configurable volume levels

## Branches

| Branch | Description |
|--------|-------------|
| `main` | English-only version |
| `chinese-support` | Chinese support: bilingual UI, Chinese font rendering, Pinyin input method |

## Hardware

- M5Cardputer (ESP32-S3, 240x135 LCD, 56-key keyboard)
- MicroSD card (optional, for message history and pinyin dictionary)

## Build & Flash

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or IDE plugin)

### English Version (main branch)

```bash
git checkout main
pio run -t upload
```

### Chinese Version (chinese-support branch)

```bash
git checkout chinese-support
pio run -t upload
```

**Additional step:** Copy the pinyin dictionary to SD card:

```
SD card root/
└── chat/
    └── pinyin_dict.txt    ← copy from data/chat/pinyin_dict.txt
```

Without this file, Pinyin input will be unavailable (English input still works).

## Usage

### Navigation

| Key | Action |
|-----|--------|
| `;` / `.` | Up / Dn |
| `,` / `/` | Lt / Rt (menu) |
| `Enter` | Select / Send |
| `` ` `` | Back to menu |
| `Del` | Delete item |
| `Fn` + `;` / `.` | Scroll chat Up/Dn |

### Pairing

1. Both devices go to **Pair** menu
2. Devices auto-discover and exchange keys
3. Once paired, select **Chat** to start messaging

### Chat

- Type message and press `Enter` to send
- `Fn` + `;` / `.` to scroll chat history
- Message status: `+` = delivered, `?` = pending, `X` = failed

### Settings

- Sound: ON/OFF
- Volume: Mute/Low/Med/High
- Screen timeout: 5m/10m/15m/30m
- Sleep timeout: 15m/30m/60m/Off
- Language: EN/中文 *(chinese-support branch only)*
- Battery level display

## Chinese Version Extras

### Switch UI Language

Settings → **Lang: EN** → press Enter → **Lang: 中文**

All menus, prompts, and status text will switch to Chinese.

### Pinyin Input

1. In Chat, press `Fn` + `Space` to toggle CN/EN input mode
2. A "中" indicator appears at the input line
3. Type pinyin letters (e.g. `ni`), candidates appear above input:
   ```
   ni: 1.你 2.呢 3.泥 4.逆 5.拟
   ```
4. Press `1`-`5` to select a candidate, or `Space` to pick the first one
5. `;` / `.` to page Up/Dn through candidates
6. `Del` to delete pinyin letters
7. `Fn` + `Space` again to switch back to English input

If no pinyin dictionary is found on SD card, a warning toast will appear when trying to activate Pinyin input.

## Project Structure

```
src/
├── main/
│   ├── main.cpp          # Entry point
│   ├── ChatApp.h/cpp     # Application logic & state machine
│   ├── UI.h/cpp          # Display rendering
│   ├── Comm.h/cpp        # ESP-NOW communication
│   ├── Crypto.h/cpp      # ECDH + AES-128-GCM
│   ├── Storage.h/cpp     # NVS persistence
│   ├── MsgStore.h/cpp    # SD card message storage
│   ├── PowerManager.h/cpp# Screen & sleep management
│   ├── Lang.h/cpp        # Bilingual string table (CN branch)
│   └── PinyinIME.h/cpp   # Pinyin input method (CN branch)
└── shared/
    └── Protocol.h        # Packet format & constants
```

## References

- [M5Cardputer](https://github.com/m5stack/M5Cardputer) - Official M5Stack Cardputer library

## License

MIT
