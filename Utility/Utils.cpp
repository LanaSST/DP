#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <algorithm>
#include "Utils.h"

namespace Utils {

    void PrintConfig(const ini_parser& parser)
    {
        const auto& cfg = parser.data();
        if (cfg.empty()) {
            std::cout << "(конфигурация пуста)\n";
            return;
        }

        std::cout << "===== CONFIG =====\n";
        for (const auto& kv : cfg) {
            if (kv.first.find("Password") != std::string::npos)
                std::cout << kv.first << " = ****\n";
            else
                std::cout << kv.first << " = " << kv.second << '\n';
        }
        std::cout << "==================\n";
    }

    namespace fs = std::filesystem;

    // Возвращает расширение файла в нижнем регистре с точкой: ".txt"
    std::string GetLowerExtension(const fs::path& p) {
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        return ext;
    }

    // Нормализует расширение из списка: добавляет точку, приводит к нижнему регистру
    std::string NormalizeExtension(std::string ext) {
        std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        if (!ext.empty() && ext.front() != '.') {
            ext.insert(ext.begin(), '.');
        }
        return ext;
    }

    // Читает файл целиком в строку
    bool ReadTextFile(const fs::path& path, std::string& outContent) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) {
            return false;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        outContent = ss.str();
        return true;
    }

    // Рекурсивно обходит каталог и читает все файлы с указанными расширениями.
    // Возвращает список пар (путь, содержимое).
    std::list<std::pair<fs::path, std::string> > LoadFiles(const fs::path& DirPath, const std::list<std::string>& listExtFile)
    {
        std::list<std::pair<fs::path, std::string>> result;

        // Предварительно нормализуем расширения
        std::vector<std::string> normExts;
        normExts.reserve(listExtFile.size());
        for (const auto& e : listExtFile) {
            normExts.push_back(NormalizeExtension(e));
        }

        if (!fs::exists(DirPath) || !fs::is_directory(DirPath)) {
            std::cerr << "Not a directory: " << DirPath << std::endl;
            return result;
        }

        // skip_permission_denied — не падать на недоступных подкаталогах
        auto options = fs::directory_options::skip_permission_denied;

        for (auto it = fs::recursive_directory_iterator(DirPath, options);
            it != fs::recursive_directory_iterator(); ++it)
        {
            // Пропускаем всё, что не обычный файл (каталоги, симлинки и т.п.)
            if (!it->is_regular_file()) {
                continue;
            }

            std::string ext = GetLowerExtension(it->path());

            // Проверяем, входит ли расширение в список
            bool matched = std::find(normExts.begin(), normExts.end(), ext) != normExts.end();
            if (!matched) {
                continue;
            }

            std::string content;
            if (ReadTextFile(it->path(), content)) {
                result.emplace_back(it->path(), std::move(content));
            } else {
                std::cerr << "Failed to read: " << it->path() << std::endl;
            }
        }

        return result;
    }

}