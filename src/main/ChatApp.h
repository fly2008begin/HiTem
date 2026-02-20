#pragma once
#include "UI.h"
#include "Comm.h"
#include "Crypto.h"
#include "Storage.h"
#include "MsgStore.h"
#include "PowerManager.h"
#include <M5Cardputer.h>

enum class AppState {
    MENU,
    PAIRING,
    DEVICE_LIST,
    CHATTING,
    MSG_HISTORY,
    HELP,
    SETTINGS,
    RANGE_TEST,
};

class ChatApp {
public:
    void begin();
    void update(); // call from loop()

private:
    // State handlers
    void handleMenu();
    void handlePairing();
    void handleDeviceList();
    void handleChatting();
    void handleMsgHistory();
    void handleHelp();
    void handleSettings();
    void handleRangeTest();

    // Packet processing
    void onPacket(const ReceivedPacket& pkt);
    void handlePairReq(const ReceivedPacket& pkt, const PacketHeader* hdr);
    void handlePairResp(const ReceivedPacket& pkt, const PacketHeader* hdr);
    void handlePairConfirm(const ReceivedPacket& pkt, const PacketHeader* hdr);
    void handleTextMsg(const ReceivedPacket& pkt, const PacketHeader* hdr);
    void handleAck(const PacketHeader* hdr);
    void handleRangePong(const PacketHeader* hdr);

    // Send helpers
    void sendTextMessage(const std::string& text);
    void sendAck(const uint8_t mac[6], const char code[PAIRING_CODE_LEN], uint16_t seq);

    // Sound
    void playNotification();

    // Keyboard input
    void processKeyboard();

    AppState _state;
    int      _menuSel;
    int      _devListSel;
    bool     _confirmDelete;
    int      _inputCursorPos;

    static constexpr int _menuItemCount = 7;

    // Chat state
    PairedDevice _chatPeer;
    std::vector<ChatMessage> _messages;
    std::string _inputBuf;
    int  _scrollOffset;
    bool _cursorOn;
    uint32_t _cursorTimer;

    // Message history state
    int  _histDevSel;       // device selection in history list
    bool _histViewing;      // true = viewing messages, false = selecting device
    std::vector<ChatMessage> _histMessages;
    int  _histScrollOffset;
    bool _histConfirmDelete;

    // Settings state
    int  _settingsSel;
    bool _soundEnabled;
    uint8_t _volume;
    uint16_t _screenTimeout;
    uint16_t _sleepTimeout;

    // Pairing state
    uint8_t _myPubKey[ECDH_PUBKEY_SIZE];
    char    _pairingStatus[64];
    bool    _pairWaitResp;
    uint8_t _pairPeerMac[6];
    char    _pairPeerCode[PAIRING_CODE_LEN + 1];

    // Modules
    UI           _ui;
    Comm         _comm;
    Crypto       _crypto;
    Storage      _storage;
    MsgStore     _msgStore;
    PowerManager _powerMgr;

    char _myCode[PAIRING_CODE_LEN + 1];
    bool _needRedraw;
    int  _unreadCount;

    // Range test state
    PairedDevice _rangePeer;
    bool _rangeActive = false;
    uint32_t _rangeLastPingMs = 0;
    uint32_t _rangeLastPongMs = 0;
    int _rangeSentCount = 0;
    int _rangeRecvCount = 0;
    bool _rangeSignalLost = false;
    static constexpr uint32_t RANGE_PING_INTERVAL_MS = 1000;
    static constexpr uint32_t RANGE_TIMEOUT_MS = 3000;

    // Typewriter effect
    bool _typingActive = false;
    std::string _typingFullText;
    int _typingCharIndex = 0;
    uint32_t _typingLastCharMs = 0;
    static constexpr int TYPING_SPEED_MS = 30;
};
