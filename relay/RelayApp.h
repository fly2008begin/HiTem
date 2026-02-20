#pragma once
#include "../src/shared/Protocol.h"
#include <cstdint>

class RelayApp {
public:
    void begin();
    void update();
    void onPacketReceived(const uint8_t* mac, const uint8_t* data, int len);

private:
    bool isDuplicate(const char senderCode[PAIRING_CODE_LEN], uint16_t seq);
    void markSeen(const char senderCode[PAIRING_CODE_LEN], uint16_t seq);
    uint16_t calcCRC16(const uint8_t* data, size_t len);

    // Deduplication cache
    struct SeenMsg {
        char sender_code[PAIRING_CODE_LEN];
        uint16_t seq;
        uint32_t timestamp;
    };
    static constexpr int SEEN_CACHE_SIZE = 32;
    static constexpr uint32_t SEEN_CACHE_TIMEOUT_MS = 60000; // 1 minute
    SeenMsg _seenCache[SEEN_CACHE_SIZE];
    int _seenCacheIdx;

    // LED pin (board-specific, will be set in begin())
    int _ledPin;
};
