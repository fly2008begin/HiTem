#pragma once
#include "Protocol.h"
#include <cstdint>
#include <functional>

struct ReceivedPacket {
    uint8_t  senderMac[6];
    uint8_t  data[MAX_PACKET_SIZE];
    size_t   len;
};

// Callback type for received packets (called from main loop, not ISR)
using PacketCallback = std::function<void(const ReceivedPacket& pkt)>;
using AckFailCallback = std::function<void(uint16_t seq)>;

class Comm {
public:
    bool begin(const char myCode[PAIRING_CODE_LEN]);
    void setMyCode(const char myCode[PAIRING_CODE_LEN]);

    // Send packet (builds header + payload)
    bool sendBroadcast(uint8_t msg_type, const uint8_t* payload, size_t payload_len);
    bool sendUnicast(const uint8_t mac[6], const char destCode[PAIRING_CODE_LEN],
                     uint8_t msg_type, const uint8_t* payload, size_t payload_len);

    // Process receive queue (call from loop)
    void processReceiveQueue();

    // Register callback
    void onPacketReceived(PacketCallback cb);
    void onAckFailed(AckFailCallback cb);

    // ACK tracking
    uint16_t getNextSeq();
    bool isAckPending(uint16_t seq);
    void markAcked(uint16_t seq);
    void checkRetries(); // call from loop to handle retries

private:
    bool buildAndSend(const uint8_t* mac, const char destCode[PAIRING_CODE_LEN],
                      uint8_t msg_type, const uint8_t* payload, size_t payload_len);
    uint16_t calcCRC16(const uint8_t* data, size_t len);

    char _myCode[PAIRING_CODE_LEN + 1];
    uint16_t _seqCounter;
    PacketCallback _callback;
    AckFailCallback _ackFailCb;

    // Pending ACK tracking for retries
    struct PendingMsg {
        bool     active;
        uint16_t seq;
        uint8_t  mac[6];
        uint8_t  packet[MAX_PACKET_SIZE];
        size_t   packet_len;
        uint8_t  retries;
        uint32_t sent_time;
    };
    static constexpr int MAX_PENDING = 4;
    PendingMsg _pending[MAX_PENDING];
};
