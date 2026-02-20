#include "PinyinIME.h"
#include <SD.h>
#include <map>

void PinyinIME::begin() {
    _pinyin = "";
    _candidates.clear();
    _currentPage = 0;
    _dictLoaded = false;
    loadDictionary();
}

void PinyinIME::loadDictionary() {
    if (!SD.begin()) {
        Serial.println("PinyinIME: SD.begin() failed");
        return;
    }

    File file = SD.open("/chat/pinyin_dict.txt", FILE_READ);
    if (!file) {
        Serial.println("PinyinIME: Failed to open /chat/pinyin_dict.txt");
        return;
    }

    Serial.println("PinyinIME: Loading dictionary...");
    int lineCount = 0;
    int entryCount = 0;

    // Format: pinyin char1 char2 char3 ...
    while (file.available()) {
        String line = file.readStringUntil('\n');
        lineCount++;
        line.trim();
        if (line.length() == 0 || line.startsWith("#")) {
            continue;
        }

        // Split by space
        int firstSpace = line.indexOf(' ');
        if (firstSpace < 0) {
            continue;
        }

        String pinyin = line.substring(0, firstSpace);
        String chars = line.substring(firstSpace + 1);
        chars.trim();

        std::vector<std::string> charList;

        // Split by space - handle UTF-8 properly
        while (chars.length() > 0) {
            chars.trim();
            if (chars.length() == 0) break;

            int spacePos = chars.indexOf(' ');
            String ch;
            if (spacePos >= 0) {
                ch = chars.substring(0, spacePos);
                chars = chars.substring(spacePos + 1);
            } else {
                ch = chars;
                chars = "";
            }

            ch.trim();
            if (ch.length() > 0) {
                charList.push_back(ch.c_str());
            }
        }

        if (charList.size() > 0) {
            _dict[pinyin.c_str()] = charList;
            entryCount++;

            // Debug: print first entry
            if (entryCount == 1) {
                Serial.printf("PinyinIME: First entry '%s' has %d candidates: ", pinyin.c_str(), charList.size());
                for (int i = 0; i < std::min(3, (int)charList.size()); i++) {
                    Serial.printf("'%s' ", charList[i].c_str());
                }
                Serial.println();
            }
        }
    }

    file.close();
    _dictLoaded = (_dict.size() > 0);

    Serial.printf("PinyinIME: Loaded %d entries from %d lines\n", entryCount, lineCount);
    Serial.printf("PinyinIME: Dictionary size: %d\n", _dict.size());
}

void PinyinIME::inputLetter(char c) {
    if (c >= 'a' && c <= 'z') {
        _pinyin += c;
        updateCandidates();
    }
}

void PinyinIME::backspace() {
    if (_pinyin.length() > 0) {
        _pinyin.pop_back();
        updateCandidates();
    }
}

void PinyinIME::clear() {
    _pinyin = "";
    _candidates.clear();
    _currentPage = 0;
}

void PinyinIME::updateCandidates() {
    _candidates.clear();
    _currentPage = 0;

    if (_pinyin.empty() || !_dictLoaded) {
        return;
    }

    auto it = _dict.find(_pinyin);
    if (it != _dict.end()) {
        _candidates = it->second;
    }
}

std::string PinyinIME::selectCandidate(int index) {
    std::vector<std::string> visible = getVisibleCandidates();
    if (index >= 0 && index < visible.size()) {
        std::string selected = visible[index];
        clear();
        return selected;
    }
    return "";
}

void PinyinIME::pageUp() {
    if (_currentPage > 0) {
        _currentPage--;
    }
}

void PinyinIME::pageDown() {
    if (_currentPage < getTotalPages() - 1) {
        _currentPage++;
    }
}

int PinyinIME::getTotalPages() const {
    if (_candidates.empty()) {
        return 0;
    }
    return (_candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
}

std::vector<std::string> PinyinIME::getVisibleCandidates() const {
    std::vector<std::string> visible;
    int start = _currentPage * CANDIDATES_PER_PAGE;
    int end = std::min(start + CANDIDATES_PER_PAGE, (int)_candidates.size());

    for (int i = start; i < end; i++) {
        visible.push_back(_candidates[i]);
    }

    return visible;
}
