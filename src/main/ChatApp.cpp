#include "ChatApp.h"
#include <cstring>
#include <cstdio>

void ChatApp::begin() {
    _storage.begin();
    _storage.getMyCode(_myCode);

    _crypto.begin();

    _comm.begin(_myCode);
    _comm.onPacketReceived([this](const ReceivedPacket& pkt) {
        onPacket(pkt);
    });
    _comm.onAckFailed([this](uint16_t seq) {
        for (auto& m : _messages) {
            if (m.dir == MsgDir::SENT && m.seq == seq && m.ack == AckStatus::PENDING) {
                m.ack = AckStatus::FAILED;
                _needRedraw = true;
                break;
            }
        }
    });

    _ui.begin(&M5Cardputer.Display);
    _msgStore.begin();

    // Warm up speaker so first tone() doesn't get swallowed
    M5Cardputer.Speaker.setVolume(0);
    M5Cardputer.Speaker.tone(1000, 10);
    delay(15);
    M5Cardputer.Speaker.stop();

    _state = AppState::MENU;
    _menuSel = 0;
    _devListSel = 0;
    _scrollOffset = 0;
    _cursorOn = true;
    _cursorTimer = millis();
    _needRedraw = true;
    _unreadCount = 0;
    _pairWaitResp = false;
    _confirmDelete = false;
    _inputCursorPos = 0;
    _histDevSel = 0;
    _histViewing = false;
    _histScrollOffset = 0;
    _histConfirmDelete = false;
    _settingsSel = 0;
    _soundEnabled = _storage.getSoundEnabled();
    _volume = _storage.getVolume();
    _screenTimeout = _storage.getScreenTimeout();
    _sleepTimeout = _storage.getSleepTimeout();
    _language = _storage.getLanguage();
    _pinyinMode = false;
    memset(_pairingStatus, 0, sizeof(_pairingStatus));

    // Initialize language
    Lang::setLanguage((_language == 0) ? Language::EN : Language::CN);

    // Initialize Pinyin IME
    _pinyinIME.begin();

    _powerMgr.begin(_screenTimeout, _sleepTimeout);
}

void ChatApp::update() {
    M5Cardputer.update();
    _comm.processReceiveQueue();
    _comm.checkRetries();
    processKeyboard();
    _powerMgr.update();

    // Skip UI drawing when screen is off
    if (_powerMgr.isScreenOff()) return;

    switch (_state) {
        case AppState::MENU:        handleMenu(); break;
        case AppState::PAIRING:     handlePairing(); break;
        case AppState::DEVICE_LIST: handleDeviceList(); break;
        case AppState::CHATTING:    handleChatting(); break;
        case AppState::MSG_HISTORY: handleMsgHistory(); break;
        case AppState::HELP:        handleHelp(); break;
        case AppState::SETTINGS:    handleSettings(); break;
        case AppState::RANGE_TEST:  handleRangeTest(); break;
    }
}

// --- Keyboard input ---
// Fn+; = Up, Fn+. = Down, Fn+, = Left, Fn+/ = Right
static bool isFnArrow(const Keyboard_Class::KeysState& keys, char& arrow) {
    if (!keys.fn) return false;
    for (char c : keys.word) {
        if (c == ';') { arrow = 'U'; return true; }
        if (c == '.') { arrow = 'D'; return true; }
        if (c == ',') { arrow = 'L'; return true; }
        if (c == '/') { arrow = 'R'; return true; }
    }
    return false;
}

// Direct ;/. navigation helper for non-chat screens
static bool isDirectNav(const Keyboard_Class::KeysState& keys, char& dir) {
    for (char c : keys.word) {
        if (c == ';') { dir = 'U'; return true; }
        if (c == '.') { dir = 'D'; return true; }
    }
    return false;
}

/// Menu-specific: ; and , = left, . and / = right for horizontal menu
static bool isMenuNav(const Keyboard_Class::KeysState& keys, char& dir) {
    for (char c : keys.word) {
        if (c == ';' || c == ',') { dir = 'L'; return true; }
        if (c == '.' || c == '/') { dir = 'R'; return true; }
    }
    return false;
}

void ChatApp::processKeyboard() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed())
        return;

    // Any keypress resets power activity timer and wakes screen
    _powerMgr.resetActivity();
    _needRedraw = true;

    Keyboard_Class::KeysState keys = M5Cardputer.Keyboard.keysState();
    char arrow = 0;
    bool hasArrow = isFnArrow(keys, arrow);
    char navDir = 0;
    bool hasNav = isDirectNav(keys, navDir);

    switch (_state) {
    case AppState::MENU: {
        if (keys.enter) {
            if (_menuSel == 0) {
                // Chat — open first paired device
                PairedDevice dev;
                if (_storage.getDevice(0, dev)) {
                    _chatPeer = dev;
                    _state = AppState::CHATTING;
                    _messages.clear();
                    _inputBuf.clear();
                    _inputCursorPos = 0;
                    _scrollOffset = 0;
                    _unreadCount = 0;
                    _msgStore.loadMessages(_myCode, _chatPeer.code, _messages);
                }
            } else if (_menuSel == 1) {
                _state = AppState::DEVICE_LIST;
                _devListSel = 0;
                _confirmDelete = false;
            } else if (_menuSel == 2) {
                _state = AppState::PAIRING;
                _pairWaitResp = false;
                strcpy(_pairingStatus, "Broadcasting...");
                _crypto.generateKeyPair(_myPubKey);
                PairKeyPayload payload;
                memcpy(payload.pubkey, _myPubKey, ECDH_PUBKEY_SIZE);
                _comm.sendBroadcast(MSG_PAIR_REQ, (uint8_t*)&payload, sizeof(payload));
                _powerMgr.setSuppressSleep(true);
            } else if (_menuSel == 3) {
                // Range Test — use first paired device
                PairedDevice dev;
                if (_storage.getDevice(0, dev)) {
                    _rangePeer = dev;
                    _rangeActive = true;
                    _rangeSentCount = 0;
                    _rangeRecvCount = 0;
                    _rangeSignalLost = false;
                    _rangeLastPingMs = 0;
                    _rangeLastPongMs = millis();
                } else {
                    _rangeActive = false;
                }
                _state = AppState::RANGE_TEST;
                _powerMgr.setSuppressSleep(true);
            } else if (_menuSel == 4) {
                _state = AppState::MSG_HISTORY;
                _histDevSel = 0;
                _histViewing = false;
                _histMessages.clear();
                _histScrollOffset = 0;
                _histConfirmDelete = false;
            } else if (_menuSel == 5) {
                _state = AppState::HELP;
            } else if (_menuSel == 6) {
                _state = AppState::SETTINGS;
                _settingsSel = 0;
            }
            _needRedraw = true;
        }
        // Horizontal navigation: ;/. = left/right
        char menuDir = 0;
        if (isMenuNav(keys, menuDir)) {
            if (menuDir == 'L') { _menuSel = (_menuSel > 0) ? _menuSel - 1 : _menuItemCount - 1; _needRedraw = true; }
            if (menuDir == 'R') { _menuSel = (_menuSel < _menuItemCount - 1) ? _menuSel + 1 : 0; _needRedraw = true; }
        }
        if (hasArrow) {
            // Map all Fn arrows to horizontal: U/L = prev, D/R = next
            if (arrow == 'L' || arrow == 'U') { _menuSel = (_menuSel > 0) ? _menuSel - 1 : _menuItemCount - 1; _needRedraw = true; }
            if (arrow == 'R' || arrow == 'D') { _menuSel = (_menuSel < _menuItemCount - 1) ? _menuSel + 1 : 0; _needRedraw = true; }
        }
        break;
    }

    case AppState::PAIRING:
        for (char c : keys.word) {
            if (c == '`') {
                _state = AppState::MENU;
                _powerMgr.setSuppressSleep(false);
                _needRedraw = true;
                break;
            }
        }
        break;

    case AppState::DEVICE_LIST: {
        for (char c : keys.word) {
            if (c == '`') { _state = AppState::MENU; _needRedraw = true; return; }
        }

        int devCount = _storage.getDeviceCount();

        if (_confirmDelete) {
            if (keys.enter) {
                PairedDevice dev;
                if (_storage.getDevice(_devListSel, dev)) {
                    _storage.removeDevice(dev.code);
                    if (_devListSel >= _storage.getDeviceCount() && _devListSel > 0)
                        _devListSel--;
                }
                _confirmDelete = false;
                _needRedraw = true;
            }
            if (keys.del || hasArrow || hasNav || (!keys.word.empty() && keys.word[0] != '`')) {
                _confirmDelete = false;
                _needRedraw = true;
            }
            break;
        }

        if (hasNav && devCount > 0) {
            if (navDir == 'U') { _devListSel = (_devListSel > 0) ? _devListSel - 1 : devCount - 1; _needRedraw = true; }
            if (navDir == 'D') { _devListSel = (_devListSel < devCount - 1) ? _devListSel + 1 : 0; _needRedraw = true; }
        }
        if (hasArrow && devCount > 0) {
            if (arrow == 'U') { _devListSel = (_devListSel > 0) ? _devListSel - 1 : devCount - 1; _needRedraw = true; }
            if (arrow == 'D') { _devListSel = (_devListSel < devCount - 1) ? _devListSel + 1 : 0; _needRedraw = true; }
        }
        if (keys.enter && devCount > 0) {
            PairedDevice dev;
            if (_storage.getDevice(_devListSel, dev)) {
                _chatPeer = dev;
                _state = AppState::CHATTING;
                _messages.clear();
                _inputBuf.clear();
                _inputCursorPos = 0;
                _scrollOffset = 0;
                _unreadCount = 0;
                _msgStore.loadMessages(_myCode, _chatPeer.code, _messages);
                _needRedraw = true;
            }
        }
        if (keys.del && devCount > 0) {
            _confirmDelete = true;
            _needRedraw = true;
        }
        break;
    }


    case AppState::CHATTING:
        // Any keypress during typewriter effect — skip animation
        if (_typingActive) {
            _typingActive = false;
            _needRedraw = true;
        }

        // Check for Fn+Space to toggle pinyin mode
        if (keys.fn && keys.space) {
            if (!_pinyinIME.isDictionaryLoaded()) {
                // Show warning - dictionary not loaded
                // For now, just beep
                if (_soundEnabled && _volume != VOL_MUTE) {
                    M5Cardputer.Speaker.setVolume(VOLUME_VALUES[_volume]);
                    M5Cardputer.Speaker.tone(1000, 200);
                }
            } else {
                _pinyinMode = !_pinyinMode;
                if (!_pinyinMode) {
                    _pinyinIME.clear();
                }
                _needRedraw = true;
            }
            break;
        }

        // In pinyin mode with candidates, handle candidate selection
        if (_pinyinMode && !_pinyinIME.getPinyin().empty()) {
            bool handled = false;

            // Check for number keys 1-5 to select candidates
            for (char c : keys.word) {
                if (c >= '1' && c <= '5') {
                    int idx = c - '1';
                    std::string selected = _pinyinIME.selectCandidate(idx);
                    if (!selected.empty() && _inputBuf.length() + selected.length() < 200) {
                        _inputBuf.insert(_inputCursorPos, selected);
                        _inputCursorPos += selected.length();
                    }
                    _needRedraw = true;
                    handled = true;
                    break;
                }
            }
            if (handled) break;

            // ;/. for page up/down in candidates
            if (hasNav) {
                if (navDir == 'U') {
                    _pinyinIME.pageUp();
                    _needRedraw = true;
                    break;
                }
                if (navDir == 'D') {
                    _pinyinIME.pageDown();
                    _needRedraw = true;
                    break;
                }
            }

            // Space selects first candidate
            if (keys.space) {
                std::string selected = _pinyinIME.selectCandidate(0);
                if (!selected.empty() && _inputBuf.length() + selected.length() < 200) {
                    _inputBuf.insert(_inputCursorPos, selected);
                    _inputCursorPos += selected.length();
                }
                _needRedraw = true;
                break;
            }

            // Del removes pinyin letter
            if (keys.del) {
                _pinyinIME.backspace();
                _needRedraw = true;
                break;
            }

            // Letter keys add to pinyin
            if (!keys.fn) {
                for (char c : keys.word) {
                    if (c >= 'a' && c <= 'z') {
                        _pinyinIME.inputLetter(c);
                        _needRedraw = true;
                        break;
                    }
                }
            }
            break;
        }

        for (char c : keys.word) {
            if (c == '`') {
                _state = AppState::MENU;
                _pinyinMode = false;
                _pinyinIME.clear();
                _needRedraw = true;
                return;
            }
        }

        if (keys.enter && !_inputBuf.empty()) {
            sendTextMessage(_inputBuf);
            _inputBuf.clear();
            _inputCursorPos = 0;
            _pinyinMode = false;
            _pinyinIME.clear();
            _needRedraw = true;
        }
        if (keys.del && !_inputBuf.empty() && _inputCursorPos > 0) {
            _inputBuf.erase(_inputCursorPos - 1, 1);
            _inputCursorPos--;
            _needRedraw = true;
        }

        if (hasArrow) {
            if (arrow == 'L' && _inputCursorPos > 0) {
                _inputCursorPos--;
                _needRedraw = true;
            }
            if (arrow == 'R' && _inputCursorPos < (int)_inputBuf.length()) {
                _inputCursorPos++;
                _needRedraw = true;
            }
            if (arrow == 'U') {
                _scrollOffset += UI::SCROLL_STEP;
                _needRedraw = true;
            }
            if (arrow == 'D' && _scrollOffset > 0) {
                _scrollOffset -= UI::SCROLL_STEP;
                if (_scrollOffset < 0) _scrollOffset = 0;
                _needRedraw = true;
            }
            break;
        }

        // In pinyin mode, letters go to pinyin IME
        if (_pinyinMode && !keys.fn) {
            for (char c : keys.word) {
                if (c == '`') continue;
                if (c >= 'a' && c <= 'z') {
                    _pinyinIME.inputLetter(c);
                    _needRedraw = true;
                }
            }
            break;
        }

        // Normal text input
        if (!keys.fn) {
            for (char c : keys.word) {
                if (c == '`') continue;
                if (_inputBuf.length() < 200) {
                    _inputBuf.insert(_inputBuf.begin() + _inputCursorPos, c);
                    _inputCursorPos++;
                    _needRedraw = true;
                }
            }
        }
        if (keys.space && !_pinyinMode && _inputBuf.length() < 200) {
            _inputBuf.insert(_inputBuf.begin() + _inputCursorPos, ' ');
            _inputCursorPos++;
            _needRedraw = true;
        }
        break;

    case AppState::MSG_HISTORY: {
        for (char c : keys.word) {
            if (c == '`') {
                if (_histViewing) {
                    _histViewing = false;
                    _histConfirmDelete = false;
                } else {
                    _state = AppState::MENU;
                }
                _needRedraw = true;
                return;
            }
        }

        int devCount = _storage.getDeviceCount();

        if (_histViewing) {
            // Viewing messages — scroll with ;/. or Fn arrows
            if (_histConfirmDelete) {
                if (keys.enter) {
                    PairedDevice dev;
                    if (_storage.getDevice(_histDevSel, dev)) {
                        _msgStore.deleteConversation(_myCode, dev.code);
                        _histMessages.clear();
                    }
                    _histConfirmDelete = false;
                    _histViewing = false;
                    _needRedraw = true;
                } else {
                    _histConfirmDelete = false;
                    _needRedraw = true;
                }
                break;
            }
            if (hasNav) {
                if (navDir == 'U') { _histScrollOffset++; _needRedraw = true; }
                if (navDir == 'D' && _histScrollOffset > 0) { _histScrollOffset--; _needRedraw = true; }
            }
            if (hasArrow) {
                if (arrow == 'U') { _histScrollOffset++; _needRedraw = true; }
                if (arrow == 'D' && _histScrollOffset > 0) { _histScrollOffset--; _needRedraw = true; }
            }
            if (keys.del) {
                _histConfirmDelete = true;
                _needRedraw = true;
            }
        } else {
            // Selecting device
            if (hasNav && devCount > 0) {
                if (navDir == 'U') { _histDevSel = (_histDevSel > 0) ? _histDevSel - 1 : devCount - 1; _needRedraw = true; }
                if (navDir == 'D') { _histDevSel = (_histDevSel < devCount - 1) ? _histDevSel + 1 : 0; _needRedraw = true; }
            }
            if (hasArrow && devCount > 0) {
                if (arrow == 'U') { _histDevSel = (_histDevSel > 0) ? _histDevSel - 1 : devCount - 1; _needRedraw = true; }
                if (arrow == 'D') { _histDevSel = (_histDevSel < devCount - 1) ? _histDevSel + 1 : 0; _needRedraw = true; }
            }
            if (keys.enter && devCount > 0) {
                PairedDevice dev;
                if (_storage.getDevice(_histDevSel, dev)) {
                    _histMessages.clear();
                    _msgStore.loadMessages(_myCode, dev.code, _histMessages);
                    _histViewing = true;
                    _histScrollOffset = 0;
                    _needRedraw = true;
                }
            }
        }
        break;
    }


    case AppState::SETTINGS:
        for (char c : keys.word) {
            if (c == '`') { _state = AppState::MENU; _needRedraw = true; return; }
        }

        if (hasNav) {
            if (navDir == 'U') { _settingsSel = (_settingsSel > 0) ? _settingsSel - 1 : 6; _needRedraw = true; }
            if (navDir == 'D') { _settingsSel = (_settingsSel < 6) ? _settingsSel + 1 : 0; _needRedraw = true; }
        }
        if (hasArrow) {
            if (arrow == 'U') { _settingsSel = (_settingsSel > 0) ? _settingsSel - 1 : 6; _needRedraw = true; }
            if (arrow == 'D') { _settingsSel = (_settingsSel < 6) ? _settingsSel + 1 : 0; _needRedraw = true; }
        }

        if (keys.enter) {
            if (_settingsSel == 0) {
                _soundEnabled = !_soundEnabled;
                _storage.setSoundEnabled(_soundEnabled);
                _needRedraw = true;
            } else if (_settingsSel == 1) {
                _volume = (_volume + 1) % 4;
                _storage.setVolume(_volume);
                _needRedraw = true;
            } else if (_settingsSel == 2) {
                static const uint16_t scrOpts[] = {300, 600, 900, 1800};
                int idx = 0;
                for (int i = 0; i < 4; i++) {
                    if (_screenTimeout == scrOpts[i]) { idx = i; break; }
                }
                idx = (idx + 1) % 4;
                _screenTimeout = scrOpts[idx];
                _storage.setScreenTimeout(_screenTimeout);
                _powerMgr.setScreenTimeout(_screenTimeout);
                _needRedraw = true;
            } else if (_settingsSel == 3) {
                static const uint16_t slpOpts[] = {900, 1800, 3600, 0};
                int idx = 0;
                for (int i = 0; i < 4; i++) {
                    if (_sleepTimeout == slpOpts[i]) { idx = i; break; }
                }
                idx = (idx + 1) % 4;
                _sleepTimeout = slpOpts[idx];
                _storage.setSleepTimeout(_sleepTimeout);
                _powerMgr.setDeepSleepTimeout(_sleepTimeout);
                _needRedraw = true;
            } else if (_settingsSel == 4) {
                // Toggle language
                _language = (_language == 0) ? 1 : 0;
                _storage.setLanguage(_language);
                Lang::setLanguage((_language == 0) ? Language::EN : Language::CN);
                _needRedraw = true;
            } else if (_settingsSel == 6) {
                _state = AppState::MENU;
                _needRedraw = true;
            }
        }
        break;

    case AppState::HELP:
        for (char c : keys.word) {
            if (c == '`') { _state = AppState::MENU; _needRedraw = true; return; }
        }
        break;

    case AppState::RANGE_TEST:
        for (char c : keys.word) {
            if (c == '`') {
                _rangeActive = false;
                _state = AppState::MENU;
                _powerMgr.setSuppressSleep(false);
                _needRedraw = true;
                return;
            }
        }
        break;
    }
}

// --- Sound ---
void ChatApp::playNotification() {
    if (!_soundEnabled || _volume == VOL_MUTE) return;
    M5Cardputer.Speaker.setVolume(VOLUME_VALUES[_volume]);
    M5Cardputer.Speaker.tone(4000, 100);
}

// --- State handlers ---
void ChatApp::handleMenu() {
    if (!_needRedraw) return;
    _needRedraw = false;

    int batt = M5Cardputer.Power.getBatteryLevel();

    // Build menu names based on current language
    const char* menuNames[7];
    menuNames[0] = Lang::menuChat();
    menuNames[1] = Lang::menuDevices();
    menuNames[2] = Lang::menuPair();
    menuNames[3] = Lang::menuRange();
    menuNames[4] = Lang::menuHistory();
    menuNames[5] = Lang::menuHelp();
    menuNames[6] = Lang::menuSettings();

    _ui.drawMenu(_myCode, batt, _menuSel, _menuItemCount, _unreadCount, menuNames);
}

void ChatApp::handlePairing() {
    static uint32_t lastBroadcast = 0;
    if (millis() - lastBroadcast > 3000) {
        PairKeyPayload payload;
        memcpy(payload.pubkey, _myPubKey, ECDH_PUBKEY_SIZE);
        _comm.sendBroadcast(MSG_PAIR_REQ, (uint8_t*)&payload, sizeof(payload));
        lastBroadcast = millis();
    }

    if (_needRedraw) {
        _needRedraw = false;
        _ui.drawPairingScreen(_myCode, _pairingStatus);
    }
}

void ChatApp::handleDeviceList() {
    if (!_needRedraw) return;
    _needRedraw = false;

    std::vector<std::string> devNames;
    int count = _storage.getDeviceCount();
    for (int i = 0; i < count; i++) {
        PairedDevice dev;
        if (_storage.getDevice(i, dev)) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%s [%02X:%02X:%02X]",
                     dev.code, dev.mac[3], dev.mac[4], dev.mac[5]);
            devNames.push_back(buf);
        }
    }
    _ui.drawDeviceList(devNames, _devListSel, _confirmDelete);
}

void ChatApp::handleChatting() {
    // Typewriter effect tick
    if (_typingActive) {
        if (millis() - _typingLastCharMs >= (uint32_t)TYPING_SPEED_MS) {
            _typingCharIndex++;
            _typingLastCharMs = millis();
            _needRedraw = true;
            if (_typingCharIndex >= (int)_typingFullText.length()) {
                // Typing complete — finalize message
                _typingActive = false;
            }
        }
    }

    if (millis() - _cursorTimer > 500) {
        _cursorOn = !_cursorOn;
        _cursorTimer = millis();
        _needRedraw = true;
    }

    if (!_needRedraw) return;
    _needRedraw = false;

    // Build display messages: if typing is active, show partial text for last msg
    std::vector<ChatMessage> displayMsgs = _messages;
    if (_typingActive && !displayMsgs.empty()) {
        ChatMessage& last = displayMsgs.back();
        last.text = _typingFullText.substr(0, _typingCharIndex);
    }

    int batt = M5Cardputer.Power.getBatteryLevel();

    // Get pinyin candidates if in pinyin mode
    const char* pinyin = _pinyinMode ? _pinyinIME.getPinyin().c_str() : nullptr;
    const std::vector<std::string>* candidates = nullptr;
    std::vector<std::string> visibleCandidates;
    if (_pinyinMode && !_pinyinIME.getPinyin().empty()) {
        visibleCandidates = _pinyinIME.getVisibleCandidates();
        candidates = &visibleCandidates;
    }

    _ui.drawChatScreen(_myCode, batt, true, displayMsgs, _scrollOffset,
                        _inputBuf.c_str(), _inputCursorPos, _cursorOn,
                        _pinyinMode, pinyin, candidates);
}

void ChatApp::handleMsgHistory() {
    if (!_needRedraw) return;
    _needRedraw = false;

    if (!_histViewing) {
        std::vector<std::string> devNames;
        if (!_msgStore.hasSD()) {
            // No SD card — show hint via empty list with special title
            devNames.push_back("[No SD card]");
        } else {
            int count = _storage.getDeviceCount();
            for (int i = 0; i < count; i++) {
                PairedDevice dev;
                if (_storage.getDevice(i, dev)) {
                    devNames.push_back(dev.code);
                }
            }
        }
        _ui.drawDeviceList(devNames, _histDevSel, false);
    } else {
        PairedDevice dev;
        const char* peerCode = "??????";
        if (_storage.getDevice(_histDevSel, dev)) {
            peerCode = dev.code;
        }
        _ui.drawMsgHistory(peerCode, _histMessages, _histScrollOffset, _histConfirmDelete);
    }
}

void ChatApp::handleSettings() {
    if (!_needRedraw) return;
    _needRedraw = false;

    int batt = M5Cardputer.Power.getBatteryLevel();
    _ui.drawSettings(_settingsSel, _soundEnabled, _volume,
                     _screenTimeout, _sleepTimeout, batt, _language);
}

void ChatApp::handleHelp() {
    if (!_needRedraw) return;
    _needRedraw = false;
    _ui.drawHelpScreen();
}

void ChatApp::handleRangeTest() {
    if (_rangeActive) {
        uint32_t now = millis();

        // Send ping periodically
        if (now - _rangeLastPingMs >= RANGE_PING_INTERVAL_MS) {
            _comm.sendUnicast(_rangePeer.mac, _rangePeer.code,
                              MSG_RANGE_PING, nullptr, 0);
            _rangeSentCount++;
            _rangeLastPingMs = now;
            _needRedraw = true;
        }

        // Check for signal loss
        bool wasLost = _rangeSignalLost;
        _rangeSignalLost = (now - _rangeLastPongMs >= RANGE_TIMEOUT_MS);
        if (_rangeSignalLost && !wasLost) {
            // Just lost signal — beep
            if (_soundEnabled && _volume != VOL_MUTE) {
                M5Cardputer.Speaker.setVolume(VOLUME_VALUES[_volume]);
                M5Cardputer.Speaker.tone(2000, 300);
            }
            _needRedraw = true;
        }
        // Continuous beep while lost (every second)
        if (_rangeSignalLost && (now / 1000 != (now - 10) / 1000)) {
            if (_soundEnabled && _volume != VOL_MUTE) {
                M5Cardputer.Speaker.setVolume(VOLUME_VALUES[_volume]);
                M5Cardputer.Speaker.tone(2000, 150);
            }
        }
    }

    if (!_needRedraw) return;
    _needRedraw = false;

    const char* peerCode = _rangeActive ? _rangePeer.code : "------";
    _ui.drawRangeTest(peerCode, _rangeSentCount, _rangeRecvCount,
                      _rangeSignalLost, _rangeActive);
}


// --- Packet handling ---
void ChatApp::onPacket(const ReceivedPacket& pkt) {
    if (pkt.len < PACKET_HEADER_SIZE) return;
    const PacketHeader* hdr = (const PacketHeader*)pkt.data;

    if (strncmp(hdr->sender_code, _myCode, PAIRING_CODE_LEN) == 0) return;

    bool isBroadcast = true;
    for (int i = 0; i < (int)PAIRING_CODE_LEN; i++) {
        if (hdr->receiver_code[i] != '0') { isBroadcast = false; break; }
    }
    if (!isBroadcast && strncmp(hdr->receiver_code, _myCode, PAIRING_CODE_LEN) != 0) return;

    switch (hdr->msg_type) {
        case MSG_PAIR_REQ:     handlePairReq(pkt, hdr); break;
        case MSG_PAIR_RESP:    handlePairResp(pkt, hdr); break;
        case MSG_PAIR_CONFIRM: handlePairConfirm(pkt, hdr); break;
        case MSG_TEXT:         handleTextMsg(pkt, hdr); break;
        case MSG_ACK:          handleAck(hdr); break;
        case MSG_RANGE_PING: {
            // Reply with pong
            char senderCode[PAIRING_CODE_LEN + 1];
            strncpy(senderCode, hdr->sender_code, PAIRING_CODE_LEN);
            senderCode[PAIRING_CODE_LEN] = '\0';
            PairedDevice dev;
            if (_storage.getDeviceByCode(senderCode, dev)) {
                _comm.sendUnicast(pkt.senderMac, senderCode, MSG_RANGE_PONG, nullptr, 0);
            }
            break;
        }
        case MSG_RANGE_PONG:   handleRangePong(hdr); break;
    }
}

void ChatApp::handlePairReq(const ReceivedPacket& pkt, const PacketHeader* hdr) {
    if (_state != AppState::PAIRING) return;
    if (hdr->payload_len < sizeof(PairKeyPayload)) return;

    const PairKeyPayload* payload = (const PairKeyPayload*)(pkt.data + PACKET_HEADER_SIZE);

    uint8_t sharedKey[AES_KEY_SIZE];
    if (!_crypto.deriveSharedKey(payload->pubkey, sharedKey)) {
        strcpy(_pairingStatus, "Key exchange failed");
        _needRedraw = true;
        return;
    }

    PairedDevice dev;
    memset(&dev, 0, sizeof(dev));
    strncpy(dev.code, hdr->sender_code, PAIRING_CODE_LEN);
    dev.code[PAIRING_CODE_LEN] = '\0';
    memcpy(dev.mac, pkt.senderMac, 6);
    memcpy(dev.shared_key, sharedKey, AES_KEY_SIZE);
    dev.valid = true;
    _storage.addDevice(dev);

    PairKeyPayload resp;
    memcpy(resp.pubkey, _myPubKey, ECDH_PUBKEY_SIZE);
    _comm.sendUnicast(pkt.senderMac, hdr->sender_code,
                       MSG_PAIR_RESP, (uint8_t*)&resp, sizeof(resp));

    snprintf(_pairingStatus, sizeof(_pairingStatus), "Paired with %s!", dev.code);
    _needRedraw = true;
}

void ChatApp::handlePairResp(const ReceivedPacket& pkt, const PacketHeader* hdr) {
    if (_state != AppState::PAIRING) return;
    if (hdr->payload_len < sizeof(PairKeyPayload)) return;

    const PairKeyPayload* payload = (const PairKeyPayload*)(pkt.data + PACKET_HEADER_SIZE);

    uint8_t sharedKey[AES_KEY_SIZE];
    if (!_crypto.deriveSharedKey(payload->pubkey, sharedKey)) {
        strcpy(_pairingStatus, "Key exchange failed");
        _needRedraw = true;
        return;
    }

    PairedDevice dev;
    memset(&dev, 0, sizeof(dev));
    strncpy(dev.code, hdr->sender_code, PAIRING_CODE_LEN);
    dev.code[PAIRING_CODE_LEN] = '\0';
    memcpy(dev.mac, pkt.senderMac, 6);
    memcpy(dev.shared_key, sharedKey, AES_KEY_SIZE);
    dev.valid = true;
    _storage.addDevice(dev);

    _comm.sendUnicast(pkt.senderMac, hdr->sender_code,
                       MSG_PAIR_CONFIRM, nullptr, 0);

    snprintf(_pairingStatus, sizeof(_pairingStatus), "Paired with %s!", dev.code);
    _needRedraw = true;
}

void ChatApp::handlePairConfirm(const ReceivedPacket& pkt, const PacketHeader* hdr) {
    if (_state != AppState::PAIRING) return;
    snprintf(_pairingStatus, sizeof(_pairingStatus), "Confirmed: %.*s",
             (int)PAIRING_CODE_LEN, hdr->sender_code);
    _needRedraw = true;
}

void ChatApp::handleTextMsg(const ReceivedPacket& pkt, const PacketHeader* hdr) {
    char senderCode[PAIRING_CODE_LEN + 1];
    strncpy(senderCode, hdr->sender_code, PAIRING_CODE_LEN);
    senderCode[PAIRING_CODE_LEN] = '\0';

    PairedDevice dev;
    if (!_storage.getDeviceByCode(senderCode, dev)) return;

    const uint8_t* encData = pkt.data + PACKET_HEADER_SIZE;
    size_t encLen = hdr->payload_len;
    uint8_t plainBuf[MAX_PAYLOAD_SIZE];
    size_t plainLen = _crypto.decrypt(dev.shared_key, encData, encLen, plainBuf, sizeof(plainBuf));
    if (plainLen == 0) return;

    std::string text((char*)plainBuf, plainLen);

    sendAck(pkt.senderMac, senderCode, hdr->seq_num);

    // Wake screen and reset activity on incoming message
    _powerMgr.resetActivity();
    _needRedraw = true;

    // Play notification sound
    playNotification();

    ChatMessage msg;
    msg.text = text;
    msg.dir = MsgDir::RECEIVED;
    msg.ack = AckStatus::ACKED;
    msg.seq = hdr->seq_num;

    // Save to SD
    _msgStore.saveMessage(_myCode, senderCode, msg);

    // If chatting with this peer, add to live view with typewriter effect
    if (_state == AppState::CHATTING &&
        strncmp(_chatPeer.code, senderCode, PAIRING_CODE_LEN) == 0) {
        _messages.push_back(msg);
        _scrollOffset = 0; // snap to bottom on new message

        // Skip typewriter if already typing — finish previous instantly
        if (_typingActive) {
            _typingActive = false;
        }
        // Start typewriter for this new message
        _typingActive = true;
        _typingFullText = text;
        _typingCharIndex = 0;
        _typingLastCharMs = millis();

        _needRedraw = true;
    } else {
        _unreadCount++;
        _needRedraw = true;
    }
}

void ChatApp::handleAck(const PacketHeader* hdr) {
    if (hdr->payload_len < sizeof(AckPayload)) return;
    const uint8_t* raw = (const uint8_t*)hdr;
    const AckPayload* ack = (const AckPayload*)(raw + PACKET_HEADER_SIZE);

    _comm.markAcked(ack->acked_seq);

    for (auto& m : _messages) {
        if (m.dir == MsgDir::SENT && m.seq == ack->acked_seq) {
            m.ack = AckStatus::ACKED;
            _needRedraw = true;
            break;
        }
    }
}

void ChatApp::handleRangePong(const PacketHeader* hdr) {
    if (_state != AppState::RANGE_TEST || !_rangeActive) return;
    // Verify it's from our range peer
    if (strncmp(hdr->sender_code, _rangePeer.code, PAIRING_CODE_LEN) != 0) return;
    _rangeRecvCount++;
    _rangeLastPongMs = millis();
    _rangeSignalLost = false;
    _needRedraw = true;
}

// --- Send helpers ---
void ChatApp::sendTextMessage(const std::string& text) {
    uint8_t encBuf[MAX_PAYLOAD_SIZE];
    size_t encLen = _crypto.encrypt(_chatPeer.shared_key,
                                     (const uint8_t*)text.c_str(), text.length(),
                                     encBuf, sizeof(encBuf));
    if (encLen == 0) return;

    uint16_t seq = _comm.getNextSeq();
    _comm.sendUnicast(_chatPeer.mac, _chatPeer.code, MSG_TEXT, encBuf, encLen);

    ChatMessage msg;
    msg.text = text;
    msg.dir = MsgDir::SENT;
    msg.ack = AckStatus::PENDING;
    msg.seq = seq;
    _messages.push_back(msg);

    // Save to SD
    _msgStore.saveMessage(_myCode, _chatPeer.code, msg);

    _scrollOffset = 0; // snap to bottom on send
    _needRedraw = true;
}

void ChatApp::sendAck(const uint8_t mac[6], const char code[PAIRING_CODE_LEN], uint16_t seq) {
    AckPayload payload;
    payload.acked_seq = seq;
    _comm.sendUnicast(mac, code, MSG_ACK, (uint8_t*)&payload, sizeof(payload));
}
