#include "UI.h"
#include <cstdio>
#include <M5GFX.h>

// Cyber-glow color scheme
static constexpr uint16_t COL_BG       = TFT_BLACK;
static constexpr uint16_t COL_PRIMARY  = TFT_CYAN;    // titles, selected, sent
static constexpr uint16_t COL_TEXT     = TFT_WHITE;    // body text, received
static constexpr uint16_t COL_DIM      = 0x4208;       // unselected, separator
static constexpr uint16_t COL_WARN     = TFT_YELLOW;   // warnings, unread
static constexpr uint16_t COL_ERR      = TFT_RED;      // errors, failed
static constexpr uint16_t COL_PENDING  = TFT_YELLOW;

void UI::begin(M5GFX* display) {
    _display = display;
    _display->setRotation(1);
    _canvas.setColorDepth(16);
    _canvas.setPsram(true);
    _canvas.createSprite(SCREEN_W, SCREEN_H);
    _canvas.setFont(&fonts::AsciiFont8x16);
    _canvas.setTextSize(1);
}

void UI::flush() {
    _canvas.pushSprite(_display, 0, 0);
}

void UI::drawStatusBar(const char* myCode, int battPct, bool connected, int unread) {
    // Pure black background, cyan text
    _canvas.fillRect(0, 0, SCREEN_W, STATUS_BAR_H, COL_BG);
    uint16_t textCol = (battPct < 20) ? COL_ERR : COL_PRIMARY;
    _canvas.setTextColor(textCol, COL_BG);

    char buf[64];
    snprintf(buf, sizeof(buf), "ID:%s Bat:%d%% %s",
             myCode, battPct, connected ? "[OK]" : "[--]");
    _canvas.setCursor(2, 2);
    _canvas.print(buf);

    if (unread > 0) {
        char tag[8];
        snprintf(tag, sizeof(tag), "[%d]", unread);
        int tagLen = strlen(tag);
        int tagX = SCREEN_W - tagLen * CHAR_W - 2;
        _canvas.setTextColor(COL_WARN, COL_BG);
        _canvas.setCursor(tagX, 2);
        _canvas.print(tag);
    }

    // Thin separator line
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);
}

void UI::drawMenu(const char* myCode, int battPct, int selected, int itemCount, int unread) {
    _canvas.fillSprite(COL_BG);
    drawStatusBar(myCode, battPct, false, unread);

    const char* names[] = {"Chat", "Devices", "Pair", "Range", "History", "Help", "Settings"};
    const char* icons[] = {"Hi", "[=]", "}{", "<->", "[...]", "[?]", "[*]"};

    // Icon area — centered large symbol
    int iconY = STATUS_BAR_H + 10;
    _canvas.setTextColor(COL_PRIMARY, COL_BG);

    // Draw icon centered, using 2x text size for emphasis
    _canvas.setTextSize(2);
    const char* icon = icons[selected];
    int iconW = strlen(icon) * CHAR_W * 2;
    int iconX = (SCREEN_W - iconW) / 2;
    _canvas.setCursor(iconX, iconY);
    _canvas.print(icon);
    _canvas.setTextSize(1);

    // Name with left/right arrows
    int nameY = iconY + 40;
    char label[40];
    snprintf(label, sizeof(label), "< %s >", names[selected]);
    int labelW = strlen(label) * CHAR_W;
    int labelX = (SCREEN_W - labelW) / 2;
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(labelX, nameY);
    _canvas.print(label);

    // Page indicator dots
    int dotY = nameY + 22;
    int totalDotsW = itemCount * 10 - 4; // each dot ~6px + 4px gap
    int dotStartX = (SCREEN_W - totalDotsW) / 2;
    for (int i = 0; i < itemCount; i++) {
        int dx = dotStartX + i * 10;
        if (i == selected) {
            _canvas.fillCircle(dx + 3, dotY + 3, 3, COL_PRIMARY);
        } else {
            _canvas.fillCircle(dx + 3, dotY + 3, 2, COL_DIM);
        }
    }

    _canvas.pushSprite(_display, 0, 0);
}

// PLACEHOLDER_DEVICE_LIST

void UI::drawDeviceList(const std::vector<std::string>& devices, int selected,
                         bool confirmDelete) {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    _canvas.print("Devices (Esc=back Del=rm)");
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    if (devices.empty()) {
        _canvas.setTextColor(COL_DIM, COL_BG);
        _canvas.setCursor(8, STATUS_BAR_H + 20);
        _canvas.print("No paired devices");
    } else {
        int startY = STATUS_BAR_H + 2;
        for (int i = 0; i < (int)devices.size(); i++) {
            int y = startY + i * FONT_H;
            if (y + FONT_H > SCREEN_H) break;
            if (i == selected) {
                _canvas.setTextColor(COL_PRIMARY, COL_BG);
                _canvas.setCursor(2, y);
                _canvas.print("> ");
            } else {
                _canvas.setTextColor(COL_TEXT, COL_BG);
                _canvas.setCursor(2, y);
                _canvas.print("  ");
            }
            _canvas.print(devices[i].c_str());
        }
    }

    if (confirmDelete && !devices.empty()) {
        int boxW = 200, boxH = 40;
        int boxX = (SCREEN_W - boxW) / 2;
        int boxY = (SCREEN_H - boxH) / 2;
        _canvas.fillRect(boxX, boxY, boxW, boxH, COL_BG);
        _canvas.drawRect(boxX, boxY, boxW, boxH, COL_PRIMARY);
        _canvas.setTextColor(COL_TEXT, COL_BG);
        _canvas.setCursor(boxX + 8, boxY + 4);
        _canvas.print("Delete device?");
        _canvas.setTextColor(COL_WARN, COL_BG);
        _canvas.setCursor(boxX + 8, boxY + 22);
        _canvas.print("Enter=Yes Other=No");
    }

    _canvas.pushSprite(_display, 0, 0);
}

// PLACEHOLDER_PAIRING

void UI::drawPairingScreen(const char* myCode, const char* status) {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    _canvas.print("Pairing (Esc=cancel)");
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    _canvas.setTextColor(COL_TEXT, COL_BG);
    _canvas.setCursor(8, STATUS_BAR_H + 8);
    char buf[48];
    snprintf(buf, sizeof(buf), "My Code: %s", myCode);
    _canvas.print(buf);

    _canvas.setTextColor(COL_DIM, COL_BG);
    _canvas.setCursor(8, STATUS_BAR_H + 28);
    _canvas.print("Searching for peers...");

    _canvas.setTextColor(COL_WARN, COL_BG);
    _canvas.setCursor(8, STATUS_BAR_H + 52);
    _canvas.print(status);

    _canvas.pushSprite(_display, 0, 0);
}

// PLACEHOLDER_CHAT

int UI::msgLineCount(const std::string& text) const {
    int cpl = getCharsPerLine() - 3;
    if (cpl <= 0) cpl = 1;
    int len = (int)text.length();
    if (len == 0) return 1;
    return (len + cpl - 1) / cpl;
}

void UI::drawChat(const std::vector<ChatMessage>& msgs, int scrollOffset) {
    _canvas.fillRect(0, MSG_AREA_Y, SCREEN_W, MSG_AREA_H, COL_BG);

    int maxDisplayLines = MSG_AREA_H / FONT_H;
    int cpl = getCharsPerLine() - 3;
    if (cpl <= 0) cpl = 1;

    struct DisplayLine {
        const ChatMessage* msg;
        std::string text;
        bool isFirstLine;
    };
    std::vector<DisplayLine> dlines;

    for (int i = 0; i < (int)msgs.size(); i++) {
        const ChatMessage& m = msgs[i];
        int ml = msgLineCount(m.text);
        for (int ln = 0; ln < ml; ln++) {
            int start = ln * cpl;
            int len = std::min(cpl, (int)m.text.length() - start);
            DisplayLine dl;
            dl.msg = &m;
            dl.isFirstLine = (ln == 0);
            dl.text = (len > 0) ? m.text.substr(start, len) : "";
            dlines.push_back(dl);
        }
    }

    int totalDlines = (int)dlines.size();
    int endIdx = totalDlines - scrollOffset;
    int beginIdx = endIdx - maxDisplayLines;
    if (beginIdx < 0) beginIdx = 0;
    if (endIdx < 0) endIdx = 0;
    if (endIdx > totalDlines) endIdx = totalDlines;

    int y = MSG_AREA_Y;
    for (int i = beginIdx; i < endIdx && y + FONT_H <= MSG_AREA_Y + MSG_AREA_H; i++) {
        const DisplayLine& dl = dlines[i];
        const ChatMessage& m = *dl.msg;

        if (m.dir == MsgDir::SENT) {
            _canvas.setTextColor(COL_PRIMARY, COL_BG);
            std::string prefix = dl.isFirstLine ? "<< " : "   ";
            std::string line = prefix + dl.text;

            const char* ackStr = "";
            uint16_t ackCol = COL_PRIMARY;
            bool isLastLine = (i + 1 >= endIdx || dlines[i + 1].msg != dl.msg);
            if (isLastLine) {
                if (m.ack == AckStatus::PENDING) { ackStr = " ?"; ackCol = COL_PENDING; }
                else if (m.ack == AckStatus::FAILED) { ackStr = " X"; ackCol = COL_ERR; }
                else { ackStr = " +"; }
            }

            int tw = (int)(line.length() + strlen(ackStr)) * CHAR_W;
            int x = SCREEN_W - tw - 2;
            if (x < 2) x = 2;
            _canvas.setCursor(x, y);
            _canvas.print(line.c_str());
            if (isLastLine) {
                _canvas.setTextColor(ackCol, COL_BG);
                _canvas.print(ackStr);
            }
        } else {
            _canvas.setTextColor(COL_TEXT, COL_BG);
            std::string prefix = dl.isFirstLine ? ">> " : "   ";
            _canvas.setCursor(2, y);
            _canvas.print((prefix + dl.text).c_str());
        }
        y += FONT_H;
    }
}

// PLACEHOLDER_INPUT

void UI::drawInputLine(const char* text, int cursorPos, bool cursorOn) {
    int y = SCREEN_H - INPUT_LINE_H;
    // Separator line above input
    _canvas.drawFastHLine(0, y, SCREEN_W, COL_DIM);
    _canvas.fillRect(0, y + 1, SCREEN_W, INPUT_LINE_H - 1, COL_BG);

    // Cyan prompt, white text
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, y + 2);
    _canvas.print("> ");

    _canvas.setTextColor(COL_TEXT, COL_BG);

    const int prefixChars = 2;
    int maxChars = (SCREEN_W - 4) / CHAR_W - prefixChars;

    std::string full(text);
    int viewStart = 0;
    if (cursorPos > maxChars - 1) {
        viewStart = cursorPos - maxChars + 1;
    }
    std::string visible = full.substr(viewStart, maxChars);
    int cursorX = cursorPos - viewStart;

    std::string display;
    for (int i = 0; i < (int)visible.length(); i++) {
        if (i == cursorX && cursorOn) {
            display += '_';
        }
        display += visible[i];
    }
    if (cursorX >= (int)visible.length()) {
        display += cursorOn ? "_" : " ";
    }

    _canvas.print(display.c_str());
}

void UI::drawChatScreen(const char* myCode, int battPct, bool connected,
                         const std::vector<ChatMessage>& msgs, int scrollOffset,
                         const char* inputText, int cursorPos, bool cursorOn) {
    drawStatusBar(myCode, battPct, connected);
    drawChat(msgs, scrollOffset);
    drawInputLine(inputText, cursorPos, cursorOn);
    _canvas.pushSprite(_display, 0, 0);
}

// PLACEHOLDER_SETTINGS

void UI::drawSettings(int selected, bool soundOn, uint8_t volume,
                      uint16_t screenTimeoutSec, uint16_t sleepTimeoutSec,
                      int battPct) {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    _canvas.print("Settings (Esc=back)");
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    const char* volNames[] = {"Mute", "Low", "Med", "High"};
    const char* volStr = (volume <= VOL_HIGH) ? volNames[volume] : "?";

    const char* scrLabel = "15m";
    if (screenTimeoutSec <= 300) scrLabel = "5m";
    else if (screenTimeoutSec <= 600) scrLabel = "10m";
    else if (screenTimeoutSec <= 900) scrLabel = "15m";
    else scrLabel = "30m";

    const char* slpLabel = "30m";
    if (sleepTimeoutSec == 0) slpLabel = "Off";
    else if (sleepTimeoutSec <= 900) slpLabel = "15m";
    else if (sleepTimeoutSec <= 1800) slpLabel = "30m";
    else slpLabel = "60m";

    char items[6][32];
    snprintf(items[0], sizeof(items[0]), "Sound: %s", soundOn ? "ON" : "OFF");
    snprintf(items[1], sizeof(items[1]), "Volume: %s", volStr);
    snprintf(items[2], sizeof(items[2]), "Screen: %s", scrLabel);
    snprintf(items[3], sizeof(items[3]), "Sleep: %s", slpLabel);
    snprintf(items[4], sizeof(items[4]), "Battery: %d%%", battPct);
    snprintf(items[5], sizeof(items[5]), "Back");

    int startY = STATUS_BAR_H + 2;
    for (int i = 0; i < 6; i++) {
        int y = startY + i * FONT_H;
        if (y + FONT_H > SCREEN_H) break;
        if (i == selected) {
            _canvas.setTextColor(COL_PRIMARY, COL_BG);
            _canvas.setCursor(2, y);
            _canvas.print("> ");
        } else {
            _canvas.setTextColor(COL_TEXT, COL_BG);
            _canvas.setCursor(2, y);
            _canvas.print("  ");
        }
        _canvas.print(items[i]);
    }

    _canvas.pushSprite(_display, 0, 0);
}

// PLACEHOLDER_HISTORY

void UI::drawMsgHistory(const char* peerCode,
                        const std::vector<ChatMessage>& msgs, int scrollOffset,
                        bool confirmDelete) {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    char title[48];
    snprintf(title, sizeof(title), "History:%s (Esc=bk Del=clr)", peerCode);
    _canvas.print(title);
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    int areaY = STATUS_BAR_H;
    int areaH = SCREEN_H - STATUS_BAR_H;
    _canvas.fillRect(0, areaY, SCREEN_W, areaH, COL_BG);

    int maxDisplayLines = areaH / FONT_H;
    int cpl = getCharsPerLine() - 3;
    if (cpl <= 0) cpl = 1;

    struct DLine { const ChatMessage* msg; std::string text; bool first; };
    std::vector<DLine> dlines;
    for (auto& m : msgs) {
        int ml = msgLineCount(m.text);
        for (int ln = 0; ln < ml; ln++) {
            int s = ln * cpl;
            int l = std::min(cpl, (int)m.text.length() - s);
            DLine dl;
            dl.msg = &m;
            dl.first = (ln == 0);
            dl.text = (l > 0) ? m.text.substr(s, l) : "";
            dlines.push_back(dl);
        }
    }

    int total = (int)dlines.size();
    int endIdx = total - scrollOffset;
    int beginIdx = endIdx - maxDisplayLines;
    if (beginIdx < 0) beginIdx = 0;
    if (endIdx < 0) endIdx = 0;
    if (endIdx > total) endIdx = total;

    int y = areaY;
    for (int i = beginIdx; i < endIdx && y + FONT_H <= areaY + areaH; i++) {
        const DLine& dl = dlines[i];
        if (dl.msg->dir == MsgDir::SENT) {
            _canvas.setTextColor(COL_PRIMARY, COL_BG);
            std::string prefix = dl.first ? "<< " : "   ";
            std::string line = prefix + dl.text;
            int tw = (int)line.length() * CHAR_W;
            int x = SCREEN_W - tw - 2;
            if (x < 2) x = 2;
            _canvas.setCursor(x, y);
            _canvas.print(line.c_str());
        } else {
            _canvas.setTextColor(COL_TEXT, COL_BG);
            std::string prefix = dl.first ? ">> " : "   ";
            _canvas.setCursor(2, y);
            _canvas.print((prefix + dl.text).c_str());
        }
        y += FONT_H;
    }

    if (confirmDelete) {
        int boxW = 210, boxH = 40;
        int boxX = (SCREEN_W - boxW) / 2;
        int boxY = (SCREEN_H - boxH) / 2;
        _canvas.fillRect(boxX, boxY, boxW, boxH, COL_BG);
        _canvas.drawRect(boxX, boxY, boxW, boxH, COL_PRIMARY);
        _canvas.setTextColor(COL_TEXT, COL_BG);
        _canvas.setCursor(boxX + 8, boxY + 4);
        _canvas.print("Delete all messages?");
        _canvas.setTextColor(COL_WARN, COL_BG);
        _canvas.setCursor(boxX + 8, boxY + 22);
        _canvas.print("Enter=Yes Other=No");
    }

    _canvas.pushSprite(_display, 0, 0);
}

int UI::getMaxVisibleMessages() {
    return MSG_AREA_H / FONT_H;
}

void UI::drawHelpScreen() {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    _canvas.print("Help (Esc=back)");
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    const char* lines[] = {
        "Up/Dn  Nav",
        "Enter  Select/Send",
        "Esc    Back to menu",
        "Del    Delete item",
        "Fn+Up/Dn  Scroll chat",
        "Lt/Rt    Menu L/R",
    };
    int startY = STATUS_BAR_H + 2;
    for (int i = 0; i < 6; i++) {
        int y = startY + i * FONT_H;
        if (y + FONT_H > SCREEN_H) break;
        _canvas.setTextColor(COL_TEXT, COL_BG);
        _canvas.setCursor(4, y);
        _canvas.print(lines[i]);
    }

    _canvas.pushSprite(_display, 0, 0);
}

void UI::drawRangeTest(const char* peerCode, int sent, int recv,
                       bool signalLost, bool active) {
    _canvas.fillSprite(COL_BG);
    _canvas.setTextColor(COL_PRIMARY, COL_BG);
    _canvas.setCursor(2, 2);
    _canvas.print("Range Test (Esc=back)");
    _canvas.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, COL_DIM);

    int y = STATUS_BAR_H + 4;

    if (!active) {
        _canvas.setTextColor(COL_DIM, COL_BG);
        _canvas.setCursor(8, y);
        _canvas.print("No paired device");
        _canvas.setCursor(8, y + FONT_H);
        _canvas.print("Pair first, then Enter");
    } else {
        char buf[40];
        _canvas.setTextColor(COL_TEXT, COL_BG);
        _canvas.setCursor(8, y);
        snprintf(buf, sizeof(buf), "Peer: %s", peerCode);
        _canvas.print(buf);

        y += FONT_H + 4;
        _canvas.setCursor(8, y);
        snprintf(buf, sizeof(buf), "Sent: %d  Recv: %d", sent, recv);
        _canvas.print(buf);

        y += FONT_H + 4;
        int pct = (sent > 0) ? (recv * 100 / sent) : 0;
        snprintf(buf, sizeof(buf), "Success: %d%%", pct);
        _canvas.setCursor(8, y);
        _canvas.print(buf);

        y += FONT_H + 8;
        if (signalLost) {
            _canvas.setTextColor(COL_ERR, COL_BG);
            _canvas.setCursor(8, y);
            _canvas.print("!! SIGNAL LOST !!");
        } else {
            _canvas.setTextColor(COL_PRIMARY, COL_BG);
            _canvas.setCursor(8, y);
            _canvas.print("Signal OK");
        }
    }

    _canvas.pushSprite(_display, 0, 0);
}
