#include "QueryParser.h"
#include <boost/locale.hpp>
#include <algorithm>
#include <cctype>

namespace {
inline bool IsWordChar(unsigned char c) {
    return std::isalnum(c) != 0 || c >= 0x80;
}
}

QueryParser::QueryParser(std::size_t maxWordCount) : m_maxWordCount(maxWordCount) {}

std::vector<std::string> QueryParser::Parse(const std::string& rawQuery) const
{
    // Очистка
    std::string cleaned;
    cleaned.reserve(rawQuery.size());
    for (unsigned char c : rawQuery) {
        cleaned += IsWordChar(c) ? static_cast<char>(c) : ' ';
    }

    // Разделение на слова по пробелам
    std::vector<std::string> words;
    std::string w;
    auto flush = [&]() {
        if (!w.empty()) words.push_back(std::move(w));
        w.clear();
    };
    for (unsigned char c : cleaned) {
        if (c == ' ') flush();
        else w += static_cast<char>(c);
    }
    flush();

    // Приведение к нижнему регистру
    boost::locale::generator gen;
    std::locale loc = gen("");
    for (auto& word : words) {
        word = boost::locale::to_lower(word, loc);
    }

    // Ограничение количества слов
    if (words.size() > m_maxWordCount) {
        words.resize(m_maxWordCount);
    }
    return words;
}
