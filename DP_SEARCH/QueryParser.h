#ifndef QueryParser_H
#define QueryParser_H

#include <string>
#include <vector>

//Обработка запроса: очистка, приведение к нижнему регистру, разделение на слова по пробелам, ограничение до maxWordCount
class QueryParser
{
public:
    explicit QueryParser(std::size_t maxWordCount = 4);

    std::vector<std::string> Parse(const std::string& rawQuery) const;

private:
    std::size_t m_maxWordCount;
};

#endif
