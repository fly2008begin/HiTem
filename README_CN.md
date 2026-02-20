# HiTerm — 基于 ESP-NOW 的 P2P 加密聊天工具

基于 M5Cardputer 的点对点加密聊天工具，使用 ESP-NOW 协议。支持 AES-128-GCM 端到端加密、ECDH 密钥交换、赛博风格 UI，以及中继扩展通信距离。

[English](README.md)

## 功能

- **P2P 聊天** — 通过 ESP-NOW 直接设备间通信
- **端到端加密** — ECDH 密钥交换 + AES-128-GCM
- **设备配对** — 6位配对码，最多8个配对设备
- **消息记录** — 存储在 SD 卡，按会话分类
- **拉距测试** — 实时信号质量监测
- **中继支持** — 透明中继设备扩展通信距离
- **电源管理** — 屏幕超时 + 深度睡眠
- **声音提醒** — 可调音量
- **中文支持** — 双语界面、中文字体、拼音输入法

## 分支

| 分支 | 说明 |
|------|------|
| `main` | 纯英文版 |
| `chinese-support` | 中文支持：双语界面、中文字体、拼音输入法 |

## 硬件

### 聊天设备

- M5Cardputer（ESP32-S3，240x135 LCD，56键键盘）
- MicroSD 卡（可选，用于消息记录和拼音词库）

### 中继设备（可选）

- 任意 ESP32-C3 开发板（如 ESP32-C3-DevKitM-1）
- 无需屏幕和按键 — 通电即工作
- 可多级串联（最多3跳）

## 编译与刷写

### 前置条件

- [PlatformIO](https://platformio.org/)（CLI 或 IDE 插件）

### 聊天固件（M5Cardputer）

#### 英文版（main 分支）

```bash
git checkout main
pio run -e m5cardputer -t upload
```

#### 中文版（chinese-support 分支）

```bash
git checkout chinese-support
pio run -e m5cardputer -t upload
```

**额外步骤：** 将拼音词库复制到 SD 卡：

```
SD 卡根目录/
└── chat/
    └── pinyin_dict.txt    ← 从 data/chat/pinyin_dict.txt 复制
```

没有此文件时，拼音输入不可用（英文输入正常使用）。

### 中继固件（ESP32-C3）

用 USB 连接 ESP32-C3 开发板，然后：

```bash
pio run -e relay -t upload
```

默认开发板为 `esp32-c3-devkitm-1`。如使用其他 ESP32 开发板，修改 `platformio.ini` 中 `[env:relay]` 的 `board` 字段即可。

刷写完成后，中继即可使用 — 只需通电。开机时板载 LED 亮1秒表示就绪，每次转发消息时 LED 短闪。

**注意：** 中继固件兼容所有 ESP32 系列（C3/S2/S3/经典款）。修改 `platformio.ini` 中的 `board` 以匹配你的硬件。

## 使用方法

### 按键导航

| 按键 | 功能 |
|------|------|
| `;` / `.` | 上 / 下 |
| `,` / `/` | 左 / 右（菜单） |
| `Enter` | 选择 / 发送 |
| `` ` `` | 返回菜单 |
| `Del` | 删除 |
| `Fn` + `;` / `.` | 聊天上下滚动 |

### 配对

1. 两台设备都进入 **Pair（配对）** 菜单
2. 设备自动发现并交换密钥
3. 配对完成后，选择 **Chat（聊天）** 开始通信

### 聊天

- 输入消息后按 `Enter` 发送
- `Fn` + `;` / `.` 滚动聊天记录
- 消息状态：`+` = 已送达，`?` = 等待中，`X` = 发送失败

### 设置

- 声音：开/关
- 音量：静音/低/中/高
- 屏幕超时：5分/10分/15分/30分
- 睡眠超时：15分/30分/60分/关
- 语言：EN/中文（chinese-support 分支）
- 电量显示

## 中文版特有功能

### 切换界面语言

设置 → **Lang: EN** → 按 Enter → **Lang: 中文**

所有菜单、提示和状态文字将切换为中文。

### 拼音输入

1. 在聊天界面按 `Fn` + `Space` 切换中/英输入模式
2. 输入栏出现"中"标识
3. 输入拼音字母（如 `ni`），候选字出现在输入栏上方：
   ```
   ni: 1.你 2.呢 3.泥 4.逆 5.拟
   ```
4. 按 `1`-`5` 选择候选字，或按 `Space` 选第一个
5. `;` / `.` 翻页浏览更多候选字
6. `Del` 删除拼音字母
7. 再按 `Fn` + `Space` 切回英文输入

如果 SD 卡中没有拼音词库，尝试激活拼音输入时会弹出警告提示。

## 项目结构

```
src/
├── main/                    # 聊天设备固件（M5Cardputer）
│   ├── main.cpp             # 入口
│   ├── ChatApp.h/cpp        # 应用逻辑与状态机
│   ├── UI.h/cpp             # 显示渲染
│   ├── Comm.h/cpp           # ESP-NOW 通信
│   ├── Crypto.h/cpp         # ECDH + AES-128-GCM
│   ├── Storage.h/cpp        # NVS 持久化存储
│   ├── MsgStore.h/cpp       # SD 卡消息存储
│   ├── PowerManager.h/cpp   # 屏幕与睡眠管理
│   ├── Lang.h/cpp           # 双语字符串表
│   └── PinyinIME.h/cpp      # 拼音输入法
├── relay/                   # 中继设备固件（ESP32）
│   ├── main.cpp             # 入口
│   └── RelayApp.h/cpp       # 接收、去重、转发逻辑
└── shared/
    └── Protocol.h           # 数据包格式与常量
```

## 中继系统

中继是透明的消息转发器，无需任何配置 — 刷好固件通电即可。

- 聊天设备发送消息时 `hop_count = 3`
- 每个中继收到后将 `hop_count` 减1并重新广播
- `hop_count = 0` 的消息不再转发
- 中继端和聊天设备端都有去重缓存，防止重复接收
- 可在两台聊天设备之间放置多个中继以延长通信距离

## 参考

- [M5Cardputer](https://github.com/m5stack/M5Cardputer) - M5Stack Cardputer 官方库

## 许可证

MIT
