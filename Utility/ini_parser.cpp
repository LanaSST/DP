#include <fstream>
#include <stdexcept>
#include <sstream>
#include "ini_parser.h"

std::list<std::string> ini_parser::get_list(const std::string& strKey) const
{
    std::list<std::string> result;

    auto it = mapValue.find(strKey);
    if (it == mapValue.end())
        return result;  // ключа нет — возвращаем пустой список

    std::stringstream ss(it->second);
    std::string item;

    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty())
            result.push_back(item);
    }

    return result;
}

ini_parser::ini_parser(const std::string & strFileName)
{
    std::ifstream file("../../" + strFileName);
    if (!file.is_open()) {
        throw std::runtime_error("File is not found");
    }
    
    std::string line;
    std::string strSection;
    while (std::getline(file, line)) {

        // Удаление комментариев (начинаются с ;)
        size_t pos = line.find_first_of(";");
        if (pos != std::string::npos) {
            line = line.substr(0, pos);
        }
        
        // Если строка после удаления комментария пуста, переходим на следующую
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        
        // Если это секция, сохраняем и переходим на след.строку
        if (line.front() == '[' && line.back() == ']') {
            strSection = line.substr(1, line.length() - 2);
            strSection = trim(strSection);
            continue;
        }
        
        // Если это ключ = значение, добавляем в контейнер
        pos = line.find('=');
        if (pos != std::string::npos) {
            auto strKey = trim(line.substr(0, pos));
            auto strValue = trim(line.substr(pos + 1));    
            strValue = remove_quotes(strValue);
            if(!strValue.empty()) {
                mapValue.insert_or_assign((strSection.empty() ? strKey : strSection + "." + strKey), strValue);
            }
        }
    }
        
    file.close();
}

std::string ini_parser::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string ini_parser::remove_quotes(const std::string& str) const
{
    if (str.length() >= 2) {
        char first = str.front();
        char last  = str.back();
        // Парные кавычки: "..." или '...'
        if ((first == '"'  && last == '"') ||
            (first == '\'' && last == '\'')) {
            return str.substr(1, str.length() - 2);
        }
    }
    return str;
}


