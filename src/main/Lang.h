#pragma once
#include <cstdint>

enum class Language : uint8_t {
    EN = 0,
    CN = 1
};

class Lang {
public:
    static void setLanguage(Language lang);
    static Language getLanguage();

    // Menu items
    static const char* menuChat();
    static const char* menuDevices();
    static const char* menuPair();
    static const char* menuRange();
    static const char* menuHistory();
    static const char* menuHelp();
    static const char* menuSettings();

    // Status bar
    static const char* statusConnected();
    static const char* statusDisconnected();

    // Device list
    static const char* deviceListTitle();
    static const char* deviceListEmpty();
    static const char* confirmDelete();

    // Pairing
    static const char* pairingTitle();
    static const char* pairingWaiting();
    static const char* pairingSuccess();
    static const char* pairingFailed();
    static const char* pairingTimeout();

    // Chat
    static const char* chatInputPrompt();
    static const char* chatEmpty();

    // Settings
    static const char* settingsSound();
    static const char* settingsVolume();
    static const char* settingsScreenTimeout();
    static const char* settingsSleepTimeout();
    static const char* settingsLanguage();
    static const char* settingsBattery();

    static const char* soundOn();
    static const char* soundOff();

    static const char* volumeMute();
    static const char* volumeLow();
    static const char* volumeMed();
    static const char* volumeHigh();

    // Help
    static const char* helpTitle();
    static const char* helpNavigation();
    static const char* helpKeys();

    // Range test
    static const char* rangeTitle();
    static const char* rangeActive();
    static const char* rangeSignalLost();
    static const char* rangeSent();
    static const char* rangeReceived();

    // Message history
    static const char* historyTitle();
    static const char* historyEmpty();

    // Common
    static const char* back();
    static const char* select();
    static const char* delete_();
    static const char* cancel();

private:
    static Language _currentLang;
};
