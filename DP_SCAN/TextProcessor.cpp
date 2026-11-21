#include "TextProcessor.h"

#include <boost/locale.hpp>
#include <algorithm>
#include <cctype>

namespace {

inline bool IsWordChar(unsigned char c) {
    return std::isalnum(c) != 0 || c >= 0x80; // байты >127 — часть UTF-8 буквы
}

} // namespace

TextProcessor::TextProcessor(std::size_t minLen, std::size_t maxLen)
    : m_minLen(minLen), m_maxLen(maxLen)
{
}

std::map<std::string, int> TextProcessor::Process(const std::string& rawText) const
{
    std::map<std::string, int> frequencies;

    // Очистка от пунктуации, табов, переводов строк, пробелов, оставляем только «словесные» символы
    std::string cleaned;
    cleaned.reserve(rawText.size());

    for (unsigned char c : rawText) {
        if (IsWordChar(c)) {
            cleaned += static_cast<char>(c);
        } else {
            cleaned += ' '; //добавляем разделитель слов
        }
    }

    // Подсчет слов
    std::string word;
    word.reserve(m_maxLen);

    auto flush = [&]() {
        if (word.empty()) return;
        // Фильтруем по длине
        if (word.size() >= m_minLen && word.size() <= m_maxLen) {
            ++frequencies[word];
        }
        word.clear();
    };

    for (unsigned char c : cleaned) {
        if (c == ' ') {
            flush();
        } else {
            word += static_cast<char>(c);
        }
    }
    flush(); // последнее слово

    // Приведение к нижнему регистру
    // Пересборка map, т.к. ключи меняются
    std::map<std::string, int> lowerFreq;
    boost::locale::generator gen;
    std::locale loc = gen("");

    for (auto& [w, cnt] : frequencies) {
        std::string lower = boost::locale::to_lower(w, loc);
        lowerFreq[lower] += cnt;   // если разные регистры дали одно слово
    }

    return lowerFreq;
}

