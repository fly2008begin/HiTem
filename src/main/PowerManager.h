#pragma once
#include <cstdint>

class PowerManager {
public:
    void begin(uint16_t screenTimeoutSec, uint16_t deepSleepTimeoutSec);
    void update();           // call every frame
    void resetActivity();    // call on keypress or message received
    void setSuppressSleep(bool suppress) { _suppressSleep = suppress; }
    bool isScreenOff() const { return _screenOff; }

    uint16_t getScreenTimeout() const { return _screenTimeoutSec; }
    void setScreenTimeout(uint16_t sec) { _screenTimeoutSec = sec; }
    uint16_t getDeepSleepTimeout() const { return _deepSleepTimeoutSec; }
    void setDeepSleepTimeout(uint16_t sec) { _deepSleepTimeoutSec = sec; }

private:
    void screenOff();
    void screenOn();
    void enterDeepSleep();

    uint32_t _lastActivityMs = 0;
    bool _screenOff = false;
    bool _suppressSleep = false;
    uint16_t _screenTimeoutSec = 900;     // 15 min
    uint16_t _deepSleepTimeoutSec = 1800; // 30 min after screen off
};
