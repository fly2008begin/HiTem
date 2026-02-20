#pragma once
#include <string>
#include <vector>
#include <M5Cardputer.h>

class PinyinIME {
public:
    void begin();

    // Returns true if dictionary loaded successfully
    bool isDictionaryLoaded() const { return _dictLoaded; }

    // Input a pinyin letter
    void inputLetter(char c);

    // Delete last pinyin letter
    void backspace();

    // Clear current pinyin input
    void clear();

    // Get current pinyin string
    const std::string& getPinyin() const { return _pinyin; }

    // Get candidates for current pinyin
    const std::vector<std::string>& getCandidates() const { return _candidates; }

    // Select candidate by index (0-4)
    std::string selectCandidate(int index);

    // Page up/down in candidates
    void pageUp();
    void pageDown();

    // Get current page info
    int getCurrentPage() const { return _currentPage; }
    int getTotalPages() const;

    // Get visible candidates (5 per page)
    std::vector<std::string> getVisibleCandidates() const;

private:
    void loadDictionary();
    void updateCandidates();

    std::string _pinyin;
    std::vector<std::string> _candidates;
    int _currentPage;
    bool _dictLoaded;

    // Dictionary: pinyin -> list of Chinese characters
    std::map<std::string, std::vector<std::string>> _dict;

    static constexpr int CANDIDATES_PER_PAGE = 5;
};
