#ifndef Utils_H
#define Utils_H

#include <filesystem>
#include "ini_parser.h"

namespace fs = std::filesystem;

namespace Utils {

    void PrintConfig(const ini_parser& parser);

    std::string GetLowerExtension(const fs::path& p);
    
    bool ReadTextFile(const fs::path& path, std::string& outContent);

    std::list<std::pair<fs::path, std::string>> LoadFiles(const fs::path& DirPath, const std::list<std::string>& listExtFile);

}

#endif