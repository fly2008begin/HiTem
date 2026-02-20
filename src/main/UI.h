#pragma once
#include "Protocol.h"
#include <M5GFX.h>
#include <cstdint>
#include <vector>
#include <string>

enum class MsgDir { SENT, RECEIVED };
enum class AckStatus { PENDING, ACKED, FAILED };

struct ChatMessage {
    std::string text;
    MsgDir      dir;
    AckStatus   ack;
    uint16_t    seq;
};

class UI {
public:
    void begin(M5GFX* display);

    // Status bar (battPct < 20 turns bar red, unread shows message count)
    void drawStatusBar(const char* myCode, int battPct, bool connected, int unread = 0);

    // Main menu (7 items now)
    void drawMenu(const char* myCode, int battPct, int selected, int itemCount, int unread = 0, const char** menuNames = nullptr);

    // Device list (with optional delete confirmation)
    void drawDeviceList(const std::vector<std::string>& devices, int selected,
                        bool confirmDelete = false);

    // Pairing screen
    void drawPairingScreen(const char* myCode, const char* status);

    // Chat view (with word wrap)
    void drawChat(const std::vector<ChatMessage>& msgs, int scrollOffset);
    void drawInputLine(const char* text, int cursorPos, bool cursorOn, bool pinyinMode = false,
                       const char* pinyin = nullptr, const std::vector<std::string>* candidates = nullptr);

    // Full chat screen refresh
    void drawChatScreen(const char* myCode, int battPct, bool connected,
                        const std::vector<ChatMessage>& msgs, int scrollOffset,
                        const char* inputText, int cursorPos, bool cursorOn,
                        bool pinyinMode = false, const char* pinyin = nullptr,
                        const std::vector<std::string>* candidates = nullptr);

    // Settings screen
    void drawSettings(int selected, bool soundOn, uint8_t volume,
                      uint16_t screenTimeoutSec, uint16_t sleepTimeoutSec,
                      int battPct, uint8_t language = 0);

    // Message history (read-only chat view)
    void drawMsgHistory(const char* peerCode,
                        const std::vector<ChatMessage>& msgs, int scrollOffset,
                        bool confirmDelete = false);

    // Help screen
    void drawHelpScreen();

    // Range test screen
    void drawRangeTest(const char* peerCode, int sent, int recv, bool signalLost, bool active);

    // Update ACK status for a message
    void updateAckStatus(uint16_t seq, AckStatus status);

    int getMaxVisibleMessages();
    int getCharsPerLine() const { return (SCREEN_W - 8) / CHAR_W; }
    // Half-page scroll step (in display lines)
    static constexpr int SCROLL_STEP = 5;

private:
    void flush();
    // Calculate how many display lines a message takes (with word wrap)
    int msgLineCount(const std::string& text) const;

    M5GFX* _display;
    M5Canvas _canvas;

    static constexpr int SCREEN_W = 240;
    static constexpr int SCREEN_H = 135;
    static constexpr int STATUS_BAR_H = 20;
    static constexpr int INPUT_LINE_H = 20;
    static constexpr int MSG_AREA_Y = STATUS_BAR_H;
    static constexpr int MSG_AREA_H = SCREEN_H - STATUS_BAR_H - INPUT_LINE_H;
    static constexpr int FONT_H = 16;
    static constexpr int CHAR_W = 8;
};
