#include "MsgStore.h"
#include <SD.h>
#include <cstring>
#include <cstdio>

bool MsgStore::begin() {
    uint8_t ct = SD.cardType();
    _sdAvailable = (ct != CARD_NONE && ct != CARD_UNKNOWN);
    if (!_sdAvailable) {
        File root = SD.open("/");
        if (root) {
            _sdAvailable = true;
            root.close();
        }
    }
    if (_sdAvailable) {
        SD.mkdir("/chat");
        SD.mkdir("/chat/messages");
        Serial.println("MsgStore: SD ready");
    } else {
        Serial.println("MsgStore: no SD");
    }
    return _sdAvailable;
}

void MsgStore::getFilePath(const char* code1, const char* code2,
                           char* out, size_t outLen) {
    if (strcmp(code1, code2) <= 0) {
        snprintf(out, outLen, "/chat/messages/%.6s_%.6s.txt", code1, code2);
    } else {
        snprintf(out, outLen, "/chat/messages/%.6s_%.6s.txt", code2, code1);
    }
}

// Text format: "<< msg\n" for sent, ">> msg\n" for received
bool MsgStore::saveMessage(const char* myCode, const char* peerCode,
                           const ChatMessage& msg) {
    if (!_sdAvailable) return false;

    char path[64];
    getFilePath(myCode, peerCode, path, sizeof(path));

    File f = SD.open(path, FILE_APPEND);
    if (!f) return false;

    const char* prefix = (msg.dir == MsgDir::SENT) ? "<< " : ">> ";
    f.print(prefix);
    f.print(msg.text.c_str());
    f.print("\n");
    f.close();
    return true;
}
int MsgStore::loadMessages(const char* myCode, const char* peerCode,
                           std::vector<ChatMessage>& msgs, int maxCount) {
    if (!_sdAvailable) return 0;

    char path[64];
    getFilePath(myCode, peerCode, path, sizeof(path));

    File f = SD.open(path, FILE_READ);
    if (!f) return 0;

    int count = 0;
    uint16_t seq = 0;
    while (f.available() && count < maxCount) {
        String line = f.readStringUntil('\n');
        if (line.length() < 3) continue; // need at least "<< " or ">> "

        MsgDir dir;
        const char* s = line.c_str();
        if (s[0] == '<' && s[1] == '<' && s[2] == ' ') {
            dir = MsgDir::SENT;
        } else if (s[0] == '>' && s[1] == '>' && s[2] == ' ') {
            dir = MsgDir::RECEIVED;
        } else {
            continue; // skip malformed lines
        }

        ChatMessage msg;
        msg.seq = seq++;
        msg.dir = dir;
        msg.ack = AckStatus::ACKED;
        msg.text = std::string(s + 3);
        // Trim trailing \r if present (Windows line endings)
        if (!msg.text.empty() && msg.text.back() == '\r') {
            msg.text.pop_back();
        }
        msgs.push_back(msg);
        count++;
    }
    f.close();
    return count;
}

bool MsgStore::deleteConversation(const char* myCode, const char* peerCode) {
    if (!_sdAvailable) return false;

    char path[64];
    getFilePath(myCode, peerCode, path, sizeof(path));
    return SD.remove(path);
}
