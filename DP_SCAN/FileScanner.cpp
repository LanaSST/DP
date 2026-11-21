#include "FileScanner.h"
#include "Utility/MyDatabase.h"     // обёртка над conn; используем для вставки
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <syncstream>

namespace {
    std::string GetLowerExtension(const fs::path& p) {
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        return ext;
    }
    std::string NormalizeExtension(std::string ext) {
        std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        if (!ext.empty() && ext.front() != '.') ext.insert(ext.begin(), '.');
        return ext;
    }
}

FileScanner::FileScanner(ConnectionPool& dbPool, std::size_t nThreads,
                 std::size_t minWordLen, std::size_t maxWordLen)
    : m_dbPool(dbPool)
    , m_pool(nThreads)
    , m_processor(minWordLen, maxWordLen)
{
}

void FileScanner::Scan(const fs::path& DirPath,
                             const std::list<std::string>& listExtFile)
{
    if (!fs::exists(DirPath) || !fs::is_directory(DirPath)) {
        throw std::runtime_error("Directory is not exist: " + DirPath.string());
    }

    // Нормализуем расширения
    std::vector<std::string> listNormExts;
    listNormExts.reserve(listExtFile.size());
    for (const auto& e : listExtFile) {
        listNormExts.push_back(NormalizeExtension(e));
    }

    std::vector<std::future<void>> futures;

    auto options = fs::directory_options::skip_permission_denied;
    for (auto it = fs::recursive_directory_iterator(DirPath, options);
         it != fs::recursive_directory_iterator(); ++it)
    {
        // Проверка что это файл
        if (!it->is_regular_file()) {
            continue;
        }

        // Перевод с нижний регистр
        std::string ext = GetLowerExtension(it->path());
        if (std::find(listNormExts.begin(), listNormExts.end(), ext) == listNormExts.end())
            continue;

        fs::path p = it->path();
        futures.push_back(m_pool.Push([this, p] { ProcessFile(p); }));
    }

    // Дожидаемся всех задач
    for (auto& f : futures) {
        try { 
            f.get(); 
        }
        catch (const std::exception& e) {
            std::cerr << "Task failed: " << e.what() << std::endl;
        }
    }
}

void FileScanner::ProcessFile(const fs::path& path)
{
    try {
        // Чтение файла
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            std::cerr << "Cannot open: " << path << std::endl;
            ++m_failedFiles;
            return;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        std::string content = ss.str();

        // Обработка текста
        auto freqs = m_processor.Process(content);
        if (freqs.empty()) {
            ++m_processedFiles;   // файл обработан, нет валидных слов
            return;
        }

        // Запись в БД — берём соединение из пула (блокирует, если все заняты)
        auto handle = m_dbPool.Get();
        MyDatabase db(handle.Get());
        Document doc(path.filename().string(), std::move(content));
        doc.mapWordFrequencies = std::move(freqs);
        db.AddDocument(doc);

        ++m_processedFiles;
    }
    catch (const std::exception& e) {
       std::osyncstream(std::cout) << "ProcessFile (" << path << ") " << e.what() << std::endl;
        ++m_failedFiles;
    }
}
