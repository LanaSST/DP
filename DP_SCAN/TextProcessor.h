#ifndef TextProcessor_H
#define TextProcessor_H

#include <string>
#include <map>

// Обработчик текста - убирает все лишнее и считает слова
class TextProcessor
{
public:
    // minLen/maxLen — фильтр длины слова
    explicit TextProcessor(std::size_t minLen = 3, std::size_t maxLen = 32);

    // Обработка текста: очистка, приведение к нижнему регистру, подсчет слов
    std::map<std::string, int> Process(const std::string& rawText) const;

private:
    std::size_t m_minLen{3};
    std::size_t m_maxLen{32};
};

#endif
