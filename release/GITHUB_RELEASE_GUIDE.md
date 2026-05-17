# GitHub Release Guide

## Release Packages Prepared

### v1.0.0 - English Version (main branch)
Location: `release/v1.0.0-main/`
- HiTerm-M5Cardputer-v1.0.0-EN.bin (965KB)
- HiTerm-Relay-ESP32C3-v1.0.0.bin (725KB)
- RELEASE_NOTES.md

### v1.1.0 - Chinese Support (chinese-support branch)
Location: `release/v1.1.0-chinese-support/`
- HiTerm-M5Cardputer-v1.1.0-CN.bin (1.3MB)
- HiTerm-Relay-ESP32C3-v1.1.0.bin (725KB)
- pinyin_dict.txt (11KB)
- RELEASE_NOTES.md

## Creating Releases on GitHub

### Step 1: Push to GitHub

```bash
# Make sure both branches are pushed
git checkout main
git push origin main

git checkout chinese-support
git push origin chinese-support
```

### Step 2: Create v1.0.0 Release (English Version)

1. Go to your repository on GitHub
2. Click **Releases** → **Draft a new release**
3. Fill in:
   - **Tag version**: `v1.0.0`
   - **Target**: `main`
   - **Release title**: `v1.0.0 - English Version`
   - **Description**: Copy content from `release/v1.0.0-main/RELEASE_NOTES.md`
4. Attach files:
   - `HiTerm-M5Cardputer-v1.0.0-EN.bin`
   - `HiTerm-Relay-ESP32C3-v1.0.0.bin`
5. Click **Publish release**

### Step 3: Create v1.1.0 Release (Chinese Support)

1. Click **Draft a new release** again
2. Fill in:
   - **Tag version**: `v1.1.0`
   - **Target**: `chinese-support`
   - **Release title**: `v1.1.0 - Chinese Support / 中文支持版`
   - **Description**: Copy content from `release/v1.1.0-chinese-support/RELEASE_NOTES.md`
3. Attach files:
   - `HiTerm-M5Cardputer-v1.1.0-CN.bin`
   - `HiTerm-Relay-ESP32C3-v1.1.0.bin`
   - `pinyin_dict.txt`
4. Click **Publish release**

## Repository Settings

### Enable Issues
1. Go to **Settings** → **General**
2. Under **Features**, check **Issues**

### Add Topics
1. Go to repository main page
2. Click the gear icon next to **About**
3. Add topics: `esp32`, `esp-now`, `m5stack`, `m5cardputer`, `encrypted-chat`, `p2p`, `chinese-input`, `pinyin`

### Branch Protection (Optional)
1. Go to **Settings** → **Branches**
2. Click **Add rule**
3. Branch name pattern: `main`
4. Enable:
   - Require pull request reviews before merging
   - Require status checks to pass before merging
5. Click **Create**
6. Repeat for `chinese-support` branch

## Recommended Repository Description

**English:**
> P2P encrypted chat for M5Cardputer using ESP-NOW. Features end-to-end encryption, device pairing, relay support, and Chinese input (Pinyin).

**中文:**
> 基于 ESP-NOW 的 M5Cardputer 点对点加密聊天工具。支持端到端加密、设备配对、中继转发和中文拼音输入。
