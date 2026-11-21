#ifndef ini_parser_h
#define ini_parser_h

#include <string>
#include <map>
#include <list>
#include <algorithm>

class ini_parser {
public:
    ini_parser(const std::string & strFileName);

    template<typename T>
    T get_value(const std::string& strKey) const
    {
        auto it = mapValue.find(strKey);
        if (it != mapValue.end()) {
            return static_cast<T>(it->second);
        }
        return T();
    }

    std::list<std::string> get_list(const std::string& strKey) const;

    const std::map<std::string, std::string>& data() const { return mapValue; }

private:
    std::string trim(const std::string& str) const;
    std::string remove_quotes(const std::string& str) const;

private:
    using ValueMap = std::map<std::string, std::string>;
    ValueMap mapValue;
};

template<>
inline std::string ini_parser::get_value<std::string>(const std::string& strKey) const
{
    auto it = mapValue.find(strKey);
    if (it != mapValue.end()) {
        return it->second;
    }
    return "";
}

template<>
inline int ini_parser::get_value<int>(const std::string& strKey) const
{
    auto it = mapValue.find(strKey);
    if (it != mapValue.end() && !it->second.empty()) {
        try {
            if(it->second.find(".") != std::string::npos) //отбрасываем явный double
                return 0;
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return 0;
        }
    }
    return 0;
}

template<>
inline bool ini_parser::get_value<bool>(const std::string& strKey) const
{
    auto it = mapValue.find(strKey);
    if (it != mapValue.end()) {
        std::string strValue = it->second;
        std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
        if(strValue == "true" || strValue == "1")
            return true;
    }
    return false;
}

template<>
inline double ini_parser::get_value<double>(const std::string& strKey) const
{
    auto it = mapValue.find(strKey);
    if (it != mapValue.end() && !it->second.empty()) {
        try {
            if(it->second.find(".") == std::string::npos) // отбрасываем явный int
                return 0.0;
            return std::stod(it->second);
        } catch (const std::exception&) {
            return 0.0;
        }
    }
    return 0.0;
}

#endif