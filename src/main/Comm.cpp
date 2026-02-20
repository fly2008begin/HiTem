#include "Comm.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Arduino.h>
#include <cstring>

// --- Receive queue (ISR-safe) ---
static constexpr int RX_QUEUE_SIZE = 8;
static ReceivedPacket rxQueue[RX_QUEUE_SIZE];
static volatile int rxHead = 0;
static volatile int rxTail = 0;

static void espnowRecvCb(const uint8_t* mac_addr,
                          const uint8_t* data, int data_len) {
    int next = (rxHead + 1) % RX_QUEUE_SIZE;
    if (next == rxTail) return; // queue full, drop

    ReceivedPacket& pkt = rxQueue[rxHead];
    memcpy(pkt.senderMac, mac_addr, 6);
    size_t copyLen = (data_len > (int)MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : data_len;
    memcpy(pkt.data, data, copyLen);
    pkt.len = copyLen;
    rxHead = next;
}

static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool Comm::begin(const char myCode[PAIRING_CODE_LEN]) {
    memcpy(_myCode, myCode, PAIRING_CODE_LEN);
    _myCode[PAIRING_CODE_LEN] = '\0';
    _seqCounter = 0;
    memset(_pending, 0, sizeof(_pending));
    memset(_seenCache, 0, sizeof(_seenCache));
    _seenCacheIdx = 0;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Enable Long Range mode — trades bandwidth for ~2x range
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_LR);

    if (esp_now_init() != ESP_OK) return false;

    esp_now_register_recv_cb(espnowRecvCb);

    // Add broadcast peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, BROADCAST_MAC, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    return true;
}

void Comm::setMyCode(const char myCode[PAIRING_CODE_LEN]) {
    memcpy(_myCode, myCode, PAIRING_CODE_LEN);
    _myCode[PAIRING_CODE_LEN] = '\0';
}

uint16_t Comm::calcCRC16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

bool Comm::buildAndSend(const uint8_t* mac, const char destCode[PAIRING_CODE_LEN],
                         uint8_t msg_type, const uint8_t* payload, size_t payload_len,
                         uint8_t hop_count) {
    if (payload_len > MAX_PAYLOAD_SIZE) return false;

    uint8_t buf[MAX_PACKET_SIZE];
    PacketHeader* hdr = (PacketHeader*)buf;

    memcpy(hdr->sender_code, _myCode, PAIRING_CODE_LEN);
    memcpy(hdr->receiver_code, destCode, PAIRING_CODE_LEN);
    hdr->msg_type    = msg_type;
    hdr->seq_num     = _seqCounter++;
    hdr->hop_count   = hop_count;
    hdr->payload_len = payload_len;
    hdr->crc16       = 0;

    if (payload_len > 0 && payload) {
        memcpy(buf + PACKET_HEADER_SIZE, payload, payload_len);
    }

    // Calculate CRC over header (with crc16=0) + payload
    hdr->crc16 = calcCRC16(buf, PACKET_HEADER_SIZE + payload_len);

    size_t total = PACKET_HEADER_SIZE + payload_len;
    return esp_now_send(mac, buf, total) == ESP_OK;
}

bool Comm::sendBroadcast(uint8_t msg_type, const uint8_t* payload, size_t payload_len) {
    const char bcast_code[PAIRING_CODE_LEN] = {'0','0','0','0','0','0'};
    return buildAndSend(BROADCAST_MAC, bcast_code, msg_type, payload, payload_len, 0);
}

bool Comm::sendBroadcastWithHops(const char destCode[PAIRING_CODE_LEN], uint8_t msg_type,
                                   const uint8_t* payload, size_t payload_len, uint8_t hop_count) {
    return buildAndSend(BROADCAST_MAC, destCode, msg_type, payload, payload_len, hop_count);
}

bool Comm::sendUnicast(const uint8_t mac[6], const char destCode[PAIRING_CODE_LEN],
                        uint8_t msg_type, const uint8_t* payload, size_t payload_len) {
    // Ensure peer is registered
    if (!esp_now_is_peer_exist(mac)) {
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, mac, 6);
        peer.channel = 0;
        peer.encrypt = false;
        esp_now_add_peer(&peer);
    }

    bool sent = buildAndSend(mac, destCode, msg_type, payload, payload_len, 0);

    // Track for ACK if it's a text message
    if (sent && msg_type == MSG_TEXT) {
        uint16_t seq = _seqCounter - 1; // just used
        for (int i = 0; i < MAX_PENDING; i++) {
            if (!_pending[i].active) {
                _pending[i].active = true;
                _pending[i].seq = seq;
                memcpy(_pending[i].mac, mac, 6);
                // Store the full packet for retry
                size_t total = PACKET_HEADER_SIZE + payload_len;
                PacketHeader* hdr = (PacketHeader*)_pending[i].packet;
                memcpy(hdr->sender_code, _myCode, PAIRING_CODE_LEN);
                memcpy(hdr->receiver_code, destCode, PAIRING_CODE_LEN);
                hdr->msg_type = msg_type;
                hdr->seq_num = seq;
                hdr->hop_count = 0;
                hdr->payload_len = payload_len;
                hdr->crc16 = 0;
                if (payload_len > 0 && payload)
                    memcpy(_pending[i].packet + PACKET_HEADER_SIZE, payload, payload_len);
                hdr->crc16 = calcCRC16(_pending[i].packet, total);
                _pending[i].packet_len = total;
                _pending[i].retries = 0;
                _pending[i].sent_time = millis();
                break;
            }
        }
    }
    return sent;
}

void Comm::processReceiveQueue() {
    while (rxTail != rxHead) {
        ReceivedPacket& pkt = rxQueue[rxTail];
        if (_callback && pkt.len >= PACKET_HEADER_SIZE) {
            _callback(pkt);
        }
        rxTail = (rxTail + 1) % RX_QUEUE_SIZE;
    }
}

void Comm::onPacketReceived(PacketCallback cb) {
    _callback = cb;
}

void Comm::onAckFailed(AckFailCallback cb) {
    _ackFailCb = cb;
}

uint16_t Comm::getNextSeq() {
    return _seqCounter;
}

bool Comm::isAckPending(uint16_t seq) {
    for (int i = 0; i < MAX_PENDING; i++) {
        if (_pending[i].active && _pending[i].seq == seq) return true;
    }
    return false;
}

void Comm::markAcked(uint16_t seq) {
    for (int i = 0; i < MAX_PENDING; i++) {
        if (_pending[i].active && _pending[i].seq == seq) {
            _pending[i].active = false;
        }
    }
}

void Comm::checkRetries() {
    uint32_t now = millis();
    for (int i = 0; i < MAX_PENDING; i++) {
        if (!_pending[i].active) continue;
        if (now - _pending[i].sent_time >= ACK_TIMEOUT_MS) {
            if (_pending[i].retries >= ACK_MAX_RETRIES) {
                uint16_t failedSeq = _pending[i].seq;
                _pending[i].active = false; // give up
                if (_ackFailCb) _ackFailCb(failedSeq);
            } else {
                // Retry
                esp_now_send(_pending[i].mac, _pending[i].packet, _pending[i].packet_len);
                _pending[i].retries++;
                _pending[i].sent_time = now;
            }
        }
    }
}

bool Comm::isDuplicate(const char senderCode[PAIRING_CODE_LEN], uint16_t seq) {
    uint32_t now = millis();
    for (int i = 0; i < SEEN_CACHE_SIZE; i++) {
        // Skip expired entries
        if (_seenCache[i].sender_code[0] != '\0' &&
            now - _seenCache[i].timestamp > SEEN_CACHE_TIMEOUT_MS) {
            _seenCache[i].sender_code[0] = '\0';
        }

        if (_seenCache[i].sender_code[0] != '\0' &&
            strncmp(_seenCache[i].sender_code, senderCode, PAIRING_CODE_LEN) == 0 &&
            _seenCache[i].seq == seq) {
            return true;
        }
    }
    return false;
}

void Comm::markSeen(const char senderCode[PAIRING_CODE_LEN], uint16_t seq) {
    memcpy(_seenCache[_seenCacheIdx].sender_code, senderCode, PAIRING_CODE_LEN);
    _seenCache[_seenCacheIdx].seq = seq;
    _seenCache[_seenCacheIdx].timestamp = millis();
    _seenCacheIdx = (_seenCacheIdx + 1) % SEEN_CACHE_SIZE;
}
