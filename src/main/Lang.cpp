#include "Lang.h"

Language Lang::_currentLang = Language::EN;

void Lang::setLanguage(Language lang) {
    _currentLang = lang;
}

Language Lang::getLanguage() {
    return _currentLang;
}

// Menu items
const char* Lang::menuChat() {
    return (_currentLang == Language::CN) ? "聊天" : "Chat";
}

const char* Lang::menuDevices() {
    return (_currentLang == Language::CN) ? "设备" : "Devices";
}

const char* Lang::menuPair() {
    return (_currentLang == Language::CN) ? "配对" : "Pair";
}

const char* Lang::menuRange() {
    return (_currentLang == Language::CN) ? "拉距" : "Range";
}

const char* Lang::menuHistory() {
    return (_currentLang == Language::CN) ? "记录" : "History";
}

const char* Lang::menuHelp() {
    return (_currentLang == Language::CN) ? "帮助" : "Help";
}

const char* Lang::menuSettings() {
    return (_currentLang == Language::CN) ? "设置" : "Settings";
}

// Status bar
const char* Lang::statusConnected() {
    return (_currentLang == Language::CN) ? "[连]" : "[OK]";
}

const char* Lang::statusDisconnected() {
    return (_currentLang == Language::CN) ? "[--]" : "[--]";
}

// Device list
const char* Lang::deviceListTitle() {
    return (_currentLang == Language::CN) ? "已配对设备" : "Paired Devices";
}

const char* Lang::deviceListEmpty() {
    return (_currentLang == Language::CN) ? "无设备" : "No devices";
}

const char* Lang::confirmDelete() {
    return (_currentLang == Language::CN) ? "删除?" : "Delete?";
}

// Pairing
const char* Lang::pairingTitle() {
    return (_currentLang == Language::CN) ? "配对中..." : "Pairing...";
}

const char* Lang::pairingWaiting() {
    return (_currentLang == Language::CN) ? "等待对方..." : "Waiting...";
}

const char* Lang::pairingSuccess() {
    return (_currentLang == Language::CN) ? "配对成功!" : "Paired!";
}

const char* Lang::pairingFailed() {
    return (_currentLang == Language::CN) ? "配对失败" : "Failed";
}

const char* Lang::pairingTimeout() {
    return (_currentLang == Language::CN) ? "超时" : "Timeout";
}

// Chat
const char* Lang::chatInputPrompt() {
    return (_currentLang == Language::CN) ? "输入:" : "Input:";
}

const char* Lang::chatEmpty() {
    return (_currentLang == Language::CN) ? "无消息" : "No messages";
}

// Settings
const char* Lang::settingsSound() {
    return (_currentLang == Language::CN) ? "声音" : "Sound";
}

const char* Lang::settingsVolume() {
    return (_currentLang == Language::CN) ? "音量" : "Volume";
}

const char* Lang::settingsScreenTimeout() {
    return (_currentLang == Language::CN) ? "熄屏" : "Screen";
}

const char* Lang::settingsSleepTimeout() {
    return (_currentLang == Language::CN) ? "休眠" : "Sleep";
}

const char* Lang::settingsLanguage() {
    return (_currentLang == Language::CN) ? "语言" : "Lang";
}

const char* Lang::settingsBattery() {
    return (_currentLang == Language::CN) ? "电量" : "Battery";
}

const char* Lang::soundOn() {
    return (_currentLang == Language::CN) ? "开" : "ON";
}

const char* Lang::soundOff() {
    return (_currentLang == Language::CN) ? "关" : "OFF";
}

const char* Lang::volumeMute() {
    return (_currentLang == Language::CN) ? "静音" : "Mute";
}

const char* Lang::volumeLow() {
    return (_currentLang == Language::CN) ? "低" : "Low";
}

const char* Lang::volumeMed() {
    return (_currentLang == Language::CN) ? "中" : "Med";
}

const char* Lang::volumeHigh() {
    return (_currentLang == Language::CN) ? "高" : "High";
}

// Help
const char* Lang::helpTitle() {
    return (_currentLang == Language::CN) ? "帮助" : "Help";
}

const char* Lang::helpNavigation() {
    return (_currentLang == Language::CN) ? "导航" : "Navigation";
}

const char* Lang::helpKeys() {
    return (_currentLang == Language::CN) ? "按键" : "Keys";
}

// Range test
const char* Lang::rangeTitle() {
    return (_currentLang == Language::CN) ? "拉距测试" : "Range Test";
}

const char* Lang::rangeActive() {
    return (_currentLang == Language::CN) ? "测试中" : "Active";
}

const char* Lang::rangeSignalLost() {
    return (_currentLang == Language::CN) ? "信号丢失" : "Signal Lost";
}

const char* Lang::rangeSent() {
    return (_currentLang == Language::CN) ? "发送" : "Sent";
}

const char* Lang::rangeReceived() {
    return (_currentLang == Language::CN) ? "接收" : "Recv";
}

// Message history
const char* Lang::historyTitle() {
    return (_currentLang == Language::CN) ? "消息记录" : "Message History";
}

const char* Lang::historyEmpty() {
    return (_currentLang == Language::CN) ? "无记录" : "No history";
}

// Common
const char* Lang::back() {
    return (_currentLang == Language::CN) ? "返回" : "Back";
}

const char* Lang::select() {
    return (_currentLang == Language::CN) ? "选择" : "Select";
}

const char* Lang::delete_() {
    return (_currentLang == Language::CN) ? "删除" : "Delete";
}

const char* Lang::cancel() {
    return (_currentLang == Language::CN) ? "取消" : "Cancel";
}
