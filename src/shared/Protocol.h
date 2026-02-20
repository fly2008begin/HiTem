#pragma once
#include <cstdint>
#include <cstring>

// --- Message types ---
enum MsgType : uint8_t {
    MSG_TEXT         = 0x01,
    MSG_ACK          = 0x02,
    MSG_PAIR_REQ     = 0x10,
    MSG_PAIR_RESP    = 0x11,
    MSG_PAIR_CONFIRM = 0x12,
    MSG_RANGE_PING   = 0x20,
    MSG_RANGE_PONG   = 0x21,
};

// --- Fixed 20-byte packet header ---
struct __attribute__((packed)) PacketHeader {
    char     sender_code[6];   // sender pairing code (ASCII digits)
    char     receiver_code[6]; // receiver pairing code (ASCII, "000000" = broadcast)
    uint8_t  msg_type;
    uint16_t seq_num;
    uint8_t  hop_count;        // reserved for relay, MVP = 0
    uint16_t payload_len;
    uint16_t crc16;
};

static_assert(sizeof(PacketHeader) == 20, "PacketHeader must be 20 bytes");

// ESP-NOW max payload = 250 bytes
constexpr size_t PACKET_HEADER_SIZE = sizeof(PacketHeader);
constexpr size_t MAX_PAYLOAD_SIZE   = 230;
constexpr size_t MAX_PACKET_SIZE    = PACKET_HEADER_SIZE + MAX_PAYLOAD_SIZE;

// Pairing code length
constexpr size_t PAIRING_CODE_LEN = 6;

// ECDH public key size (Curve25519 = 32 bytes)
constexpr size_t ECDH_PUBKEY_SIZE = 32;

// AES-GCM parameters
constexpr size_t AES_KEY_SIZE  = 16;  // AES-128
constexpr size_t GCM_IV_SIZE   = 12;
constexpr size_t GCM_TAG_SIZE  = 16;

// ACK retry
constexpr uint8_t  ACK_MAX_RETRIES    = 3;
constexpr uint32_t ACK_TIMEOUT_MS     = 2000;

// Max paired devices
constexpr size_t MAX_PAIRED_DEVICES = 8;

// --- Payload structures ---

// Volume levels
enum VolumeLevel : uint8_t {
    VOL_MUTE = 0,
    VOL_LOW  = 1,
    VOL_MED  = 2,
    VOL_HIGH = 3,
};

// Speaker volume mapping
constexpr int VOLUME_VALUES[] = {0, 64, 160, 255};

// MSG_PAIR_REQ / MSG_PAIR_RESP payload: ECDH public key
struct __attribute__((packed)) PairKeyPayload {
    uint8_t pubkey[ECDH_PUBKEY_SIZE];
};

// MSG_ACK payload: acked sequence number
struct __attribute__((packed)) AckPayload {
    uint16_t acked_seq;
};
