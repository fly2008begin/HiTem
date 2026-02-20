#pragma once
#include "Protocol.h"
#include <cstdint>

struct PairedDevice {
    char     code[PAIRING_CODE_LEN + 1]; // null-terminated
    uint8_t  mac[6];
    uint8_t  shared_key[AES_KEY_SIZE];
    bool     valid;
};

class Storage {
public:
    void begin();

    // Own pairing code
    void getMyCode(char out[PAIRING_CODE_LEN + 1]);
    void regenerateMyCode();

    // Paired device list
    int  getDeviceCount();
    bool getDevice(int index, PairedDevice& dev);
    bool getDeviceByCode(const char* code, PairedDevice& dev);
    bool addDevice(const PairedDevice& dev);
    bool removeDevice(const char* code);

    // Sound settings
    bool getSoundEnabled();
    void setSoundEnabled(bool on);
    uint8_t getVolume(); // VolumeLevel enum
    void setVolume(uint8_t vol);

    // Power settings
    uint16_t getScreenTimeout();   // seconds, default 900 (15 min)
    void setScreenTimeout(uint16_t sec);
    uint16_t getSleepTimeout();    // seconds, default 1800 (30 min), 0=disabled
    void setSleepTimeout(uint16_t sec);

    // Language settings
    uint8_t getLanguage();         // 0=EN, 1=CN
    void setLanguage(uint8_t lang);

private:
    void generateRandomCode(char out[PAIRING_CODE_LEN + 1]);
    char _myCode[PAIRING_CODE_LEN + 1];
};
