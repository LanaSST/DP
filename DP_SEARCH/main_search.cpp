#include <iostream>
#include <vector>
#include "Utility/MyDatabase.h"
#include "Utility/ini_parser.h"
#include "Utility/Utils.h"
#include "Utility/ConnectionPool.h"
#include "QueryParser.h"

int main()
{   
    try {
        // Работа с конфигом
        ini_parser parser("config.ini");
        auto strDbHost = parser.get_value<std::string>("Database.Host");
        auto strDbPort = parser.get_value<std::string>("Database.Port");
        auto strDbName = parser.get_value<std::string>("Database.DbName");
        auto strUserName = parser.get_value<std::string>("Database.UserName");
        auto strPassword = parser.get_value<std::string>("Database.Password");

        Utils::PrintConfig(parser);

        std::vector<std::string> missing;
        if (strDbHost.empty())   missing.push_back("Database.Host");
        if (strDbPort.empty())   missing.push_back("Database.Port");
        if (strDbName.empty())   missing.push_back("Database.DbName");
        if (strUserName.empty()) missing.push_back("Database.UserName");
        if (strPassword.empty()) missing.push_back("Database.Password");

        if (!missing.empty()) {
            std::string msg = "The configuration is incorrect. The parameters are not set:";
            for (const auto& name : missing)
                msg += "\n  - " + name;
            throw std::runtime_error(msg);   
        }

        if (strDbPort.empty() || !std::all_of(strDbPort.begin(), strDbPort.end(),
                        [](unsigned char c){ return std::isdigit(c); })) {
            throw std::runtime_error("Database.Port is not a number: '" + strDbPort + "'");
        }

        // Создание БД и коннекции
        std::string connStr = "host=" + strDbHost + 
                               " port=" + strDbPort + 
                               " dbname=" + strDbName + 
                               " user=" + strUserName + 
                               " password=" + strPassword;

        ConnectionPool dbPool(connStr, 1);
        QueryParser queryParser(4);

        std::string line;
        while (true) {
            std::cout << "\nEnter search query (exit: empty line):\n> ";
            if (!std::getline(std::cin, line)) break;
            if (line.empty()) break;

            auto words = queryParser.Parse(line);
            if (words.empty()) {
                std::cout << "Invalid words in query.\n";
                continue;
            }

            auto handle = dbPool.Get();
            MyDatabase db(handle.Get());

            auto results = db.SearchDocuments(words, 10);

            if (results.empty()) {
                std::cout << "Nothing found for:";
                for (const auto& w : words) std::cout << ' ' << w;
                std::cout << "\n";
            } else {
                std::cout << "Results (sorted by relevance):\n";
                int place = 1;
                for (const auto& r : results) {
                    std::cout << "  " << place++ << ". "
                              << r.m_strDocumentName
                              << "  [relevance: " << r.m_nRelevance << "]\n";
                }
            }
        }
    }
    catch (const pqxx::unique_violation& e) {
        std::cout << "DB unique_violation: " << e.what() << std::endl;
        return 1;        
    }
    catch(const pqxx::failure& e) {
        std::cout << "DB error: " << e.what() << std::endl;
        return 1;        
    }
    catch(const std::runtime_error& e) {
        std::cout << "Runtime error: " << e.what() << std::endl;
        return 1;        
    }
    catch(const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "Unknown error" << std::endl;
        return 1;
    }
    return 0;
}