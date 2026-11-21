#include <iostream>
#include <vector>
#include "Utility/MyDatabase.h"
#include "Utility/ini_parser.h"
#include "Utils.h"
#include "TextProcessor.h"
#include "Utility/ConnectionPool.h"
#include "FileScanner.h"

int main()
{   
    try {
        // Работа с конфигом
        ini_parser parser("config.ini");
        auto strDirPath = parser.get_value<std::string>("ScanParam.Path");
        auto listExtFile = parser.get_list("ScanParam.ExtFile");
        auto strDbHost = parser.get_value<std::string>("Database.Host");
        auto strDbPort = parser.get_value<std::string>("Database.Port");
        auto strDbName = parser.get_value<std::string>("Database.DbName");
        auto strUserName = parser.get_value<std::string>("Database.UserName");
        auto strPassword = parser.get_value<std::string>("Database.Password");

        Utils::PrintConfig(parser);

        std::vector<std::string> missing;
        if (strDirPath.empty())  missing.push_back("IndexingParam.Path");
        if (listExtFile.empty()) missing.push_back("IndexingParam.ExtFile");
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

        // Создание БД и пула коннекций к ней
        std::string connStr = "host=" + strDbHost + 
                               " port=" + strDbPort + 
                               " dbname=" + strDbName + 
                               " user=" + strUserName + 
                               " password=" + strPassword;
        const unsigned hw = std::thread::hardware_concurrency();
        const std::size_t nThreads = std::max<std::size_t>(1, hw);
        std::cout << "Thread count: " << nThreads << std::endl;
        ConnectionPool dbPool(connStr, nThreads);

        {
            auto handle = dbPool.Get();
            MyDatabase db(handle.Get());
            db.CreateDatabase();
        }

        // Обработка заданной директории с файлами, запись в БД
        FileScanner scanner(dbPool, nThreads, 3, 32);

        scanner.Scan(strDirPath, listExtFile);

        std::cout << "Done. Processed: " << scanner.ProcessedFiles()
                  << ", failed: " << scanner.FailedFiles() << std::endl;
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