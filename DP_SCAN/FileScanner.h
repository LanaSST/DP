#ifndef FileScanner_H
#define FileScanner_H

#include "ThreadPool.h"
#include "Utility/ConnectionPool.h"
#include "TextProcessor.h"
#include <filesystem>
#include <atomic>
#include <map>
#include <mutex>

namespace fs = std::filesystem;

class FileScanner
{
public:
    FileScanner(ConnectionPool& dbPool,
            std::size_t nThreads,
            std::size_t minWordLen = 3,
            std::size_t maxWordLen = 32);

    // Рекурсивная индексация директории
    void Scan(const fs::path& rootDir,
                        const std::list<std::string>& extensions);

    // Статистика после завершения (потокобезопасное чтение — после IndexDirectory)
    std::size_t ProcessedFiles() const { return m_processedFiles.load(); }
    std::size_t FailedFiles()    const { return m_failedFiles.load(); }

private:
    void ProcessFile(const fs::path& path);

    ConnectionPool& m_dbPool;
    ThreadPool m_pool;
    TextProcessor m_processor;

    std::atomic<std::size_t> m_processedFiles{0};
    std::atomic<std::size_t> m_failedFiles{0};
};

#endif
