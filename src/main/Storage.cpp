#include "Storage.h"
#include <Preferences.h>
#include <esp_random.h>
#include <cstdio>
#include <cstring>

static Preferences prefs;

void Storage::begin() {
    prefs.begin("hitem", false);

    // Load or generate own pairing code
    if (prefs.isKey("mycode")) {
        String s = prefs.getString("mycode", "000000");
        strncpy(_myCode, s.c_str(), PAIRING_CODE_LEN);
        _myCode[PAIRING_CODE_LEN] = '\0';
    } else {
        regenerateMyCode();
    }
}

void Storage::getMyCode(char out[PAIRING_CODE_LEN + 1]) {
    memcpy(out, _myCode, PAIRING_CODE_LEN + 1);
}

void Storage::regenerateMyCode() {
    generateRandomCode(_myCode);
    prefs.putString("mycode", _myCode);
}

void Storage::generateRandomCode(char out[PAIRING_CODE_LEN + 1]) {
    uint32_t r = esp_random();
    r %= 1000000;
    snprintf(out, PAIRING_CODE_LEN + 1, "%06lu", (unsigned long)r);
}

int Storage::getDeviceCount() {
    return prefs.getInt("devcnt", 0);
}

bool Storage::getDevice(int index, PairedDevice& dev) {
    int count = getDeviceCount();
    if (index < 0 || index >= count) return false;

    char key[16];
    snprintf(key, sizeof(key), "dev%d", index);
    size_t len = prefs.getBytesLength(key);
    if (len != sizeof(PairedDevice)) return false;

    prefs.getBytes(key, &dev, sizeof(PairedDevice));
    return dev.valid;
}

bool Storage::getDeviceByCode(const char* code, PairedDevice& dev) {
    int count = getDeviceCount();
    for (int i = 0; i < count; i++) {
        if (getDevice(i, dev) && strncmp(dev.code, code, PAIRING_CODE_LEN) == 0) {
            return true;
        }
    }
    return false;
}

bool Storage::addDevice(const PairedDevice& dev) {
    int count = getDeviceCount();

    // Check if already exists, update in place
    PairedDevice existing;
    for (int i = 0; i < count; i++) {
        if (getDevice(i, existing) && strncmp(existing.code, dev.code, PAIRING_CODE_LEN) == 0) {
            char key[16];
            snprintf(key, sizeof(key), "dev%d", i);
            prefs.putBytes(key, &dev, sizeof(PairedDevice));
            return true;
        }
    }

    if (count >= (int)MAX_PAIRED_DEVICES) return false;

    char key[16];
    snprintf(key, sizeof(key), "dev%d", count);
    prefs.putBytes(key, &dev, sizeof(PairedDevice));
    prefs.putInt("devcnt", count + 1);
    return true;
}

bool Storage::removeDevice(const char* code) {
    int count = getDeviceCount();
    int found = -1;
    PairedDevice dev;

    for (int i = 0; i < count; i++) {
        if (getDevice(i, dev) && strncmp(dev.code, code, PAIRING_CODE_LEN) == 0) {
            found = i;
            break;
        }
    }
    if (found < 0) return false;

    // Shift remaining devices down
    for (int i = found; i < count - 1; i++) {
        char srcKey[16], dstKey[16];
        snprintf(srcKey, sizeof(srcKey), "dev%d", i + 1);
        snprintf(dstKey, sizeof(dstKey), "dev%d", i);
        PairedDevice tmp;
        prefs.getBytes(srcKey, &tmp, sizeof(PairedDevice));
        prefs.putBytes(dstKey, &tmp, sizeof(PairedDevice));
    }

    // Remove last slot
    char lastKey[16];
    snprintf(lastKey, sizeof(lastKey), "dev%d", count - 1);
    prefs.remove(lastKey);
    prefs.putInt("devcnt", count - 1);
    return true;
}

bool Storage::getSoundEnabled() {
    return prefs.getBool("snd_on", false);
}

void Storage::setSoundEnabled(bool on) {
    prefs.putBool("snd_on", on);
}

uint8_t Storage::getVolume() {
    return prefs.getUChar("snd_vol", VOL_MED);
}

void Storage::setVolume(uint8_t vol) {
    if (vol > VOL_HIGH) vol = VOL_HIGH;
    prefs.putUChar("snd_vol", vol);
}

uint16_t Storage::getScreenTimeout() {
    return prefs.getUShort("scr_tout", 900);
}

void Storage::setScreenTimeout(uint16_t sec) {
    prefs.putUShort("scr_tout", sec);
}

uint16_t Storage::getSleepTimeout() {
    return prefs.getUShort("slp_tout", 1800);
}

void Storage::setSleepTimeout(uint16_t sec) {
    prefs.putUShort("slp_tout", sec);
}

uint8_t Storage::getLanguage() {
    return prefs.getUChar("lang", 0);
}

void Storage::setLanguage(uint8_t lang) {
    prefs.putUChar("lang", lang);
}
