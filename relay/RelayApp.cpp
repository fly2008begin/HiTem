#include "RelayApp.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Arduino.h>
#include <cstring>

static RelayApp* g_relayApp = nullptr;

static void espnowRecvCb(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
    if (g_relayApp) {
        g_relayApp->onPacketReceived(mac_addr, data, data_len);
    }
}

void RelayApp::begin() {
    memset(_seenCache, 0, sizeof(_seenCache));
    _seenCacheIdx = 0;
    g_relayApp = this;

    // Detect LED pin based on board
    #if defined(CONFIG_IDF_TARGET_ESP32C3)
        _ledPin = 8; // ESP32-C3-DevKitM-1
    #elif defined(CONFIG_IDF_TARGET_ESP32S3)
        _ledPin = 21; // Common for ESP32-S3
    #elif defined(CONFIG_IDF_TARGET_ESP32S2)
        _ledPin = 15; // Common for ESP32-S2
    #else
        _ledPin = 2; // Classic ESP32
    #endif

    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, HIGH);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Enable Long Range mode
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_LR);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(espnowRecvCb);

    // Add broadcast peer
    const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, BROADCAST_MAC, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    Serial.println("Relay ready");
    delay(1000);
    digitalWrite(_ledPin, LOW);
}

void RelayApp::update() {
    // Nothing to do in main loop, all work done in callback
    delay(10);
}

void RelayApp::onPacketReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len < PACKET_HEADER_SIZE) return;

    const PacketHeader* hdr = (const PacketHeader*)data;

    // Check for duplicates
    if (isDuplicate(hdr->sender_code, hdr->seq_num)) return;
    markSeen(hdr->sender_code, hdr->seq_num);

    // Only relay MSG_TEXT with hop_count > 0
    if (hdr->msg_type != MSG_TEXT || hdr->hop_count == 0) return;

    // Verify CRC
    uint8_t buf[MAX_PACKET_SIZE];
    memcpy(buf, data, len);
    PacketHeader* checkHdr = (PacketHeader*)buf;
    uint16_t receivedCRC = checkHdr->crc16;
    checkHdr->crc16 = 0;
    uint16_t calculatedCRC = calcCRC16(buf, len);
    if (receivedCRC != calculatedCRC) return;

    // Decrement hop_count and recalculate CRC
    PacketHeader* relayHdr = (PacketHeader*)buf;
    relayHdr->hop_count--;
    relayHdr->crc16 = 0;
    relayHdr->crc16 = calcCRC16(buf, len);

    // Rebroadcast
    const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(BROADCAST_MAC, buf, len);

    // Flash LED
    digitalWrite(_ledPin, HIGH);
    delay(50);
    digitalWrite(_ledPin, LOW);
}

bool RelayApp::isDuplicate(const char senderCode[PAIRING_CODE_LEN], uint16_t seq) {
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

void RelayApp::markSeen(const char senderCode[PAIRING_CODE_LEN], uint16_t seq) {
    memcpy(_seenCache[_seenCacheIdx].sender_code, senderCode, PAIRING_CODE_LEN);
    _seenCache[_seenCacheIdx].seq = seq;
    _seenCache[_seenCacheIdx].timestamp = millis();
    _seenCacheIdx = (_seenCacheIdx + 1) % SEEN_CACHE_SIZE;
}

uint16_t RelayApp::calcCRC16(const uint8_t* data, size_t len) {
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
