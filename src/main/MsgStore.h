#pragma once
#include "UI.h"  // for ChatMessage, MsgDir, AckStatus
#include <vector>
#include <cstdint>

class MsgStore {
public:
    bool begin();  // check SD available
    bool hasSD() const { return _sdAvailable; }

    bool saveMessage(const char* myCode, const char* peerCode,
                     const ChatMessage& msg);
    int  loadMessages(const char* myCode, const char* peerCode,
                      std::vector<ChatMessage>& msgs, int maxCount = 200);
    bool deleteConversation(const char* myCode, const char* peerCode);

private:
    bool _sdAvailable = false;
    // Build file path: /chat/messages/CODE1_CODE2.txt (smaller code first)
    void getFilePath(const char* code1, const char* code2, char* out, size_t outLen);
};
