#include "PowerManager.h"
#include <M5Cardputer.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

void PowerManager::begin(uint16_t screenTimeoutSec, uint16_t deepSleepTimeoutSec) {
    _screenTimeoutSec = screenTimeoutSec;
    _deepSleepTimeoutSec = deepSleepTimeoutSec;
    _lastActivityMs = millis();
    _screenOff = false;
}

void PowerManager::update() {
    // Low battery protection: deep sleep if <5%
    int batt = M5Cardputer.Power.getBatteryLevel();
    if (batt >= 0 && batt < 5) {
        enterDeepSleep();
        return;
    }

    // Suppress screen-off and deep sleep during active operations
    if (_suppressSleep) return;

    uint32_t elapsed = millis() - _lastActivityMs;

    // Deep sleep check (screen timeout + deep sleep timeout)
    if (_screenOff && _deepSleepTimeoutSec > 0) {
        uint32_t totalMs = ((uint32_t)_screenTimeoutSec + _deepSleepTimeoutSec) * 1000UL;
        if (elapsed > totalMs) {
            enterDeepSleep();
            return;
        }
    }

    // Screen off check
    if (!_screenOff && _screenTimeoutSec > 0) {
        if (elapsed > (uint32_t)_screenTimeoutSec * 1000UL) {
            screenOff();
        }
    }
}

void PowerManager::resetActivity() {
    _lastActivityMs = millis();
    if (_screenOff) {
        screenOn();
    }
}

void PowerManager::screenOff() {
    _screenOff = true;
    M5Cardputer.Display.setBrightness(0);
}

void PowerManager::screenOn() {
    _screenOff = false;
    M5Cardputer.Display.setBrightness(80);
}

void PowerManager::enterDeepSleep() {
    M5Cardputer.Display.sleep();
    M5Cardputer.Display.waitDisplay();
    // GPIO0 = Home button, wake on low level (pressed)
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(GPIO_NUM_0, GPIO_INTR_LOW_LEVEL);
    esp_deep_sleep_start();
}
