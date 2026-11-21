#include "ConnectionPool.h"

ConnectionPool::ConnectionPool(const std::string& connStr, std::size_t size)
    : m_connStr(connStr)
{
    for (std::size_t i = 0; i < size; ++i) {
        m_free.push(std::make_shared<pqxx::connection>(connStr));
    }
}

ConnectionPool::Handle ConnectionPool::Get()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { 
        return !m_free.empty(); 
    });
    auto conn = m_free.front();
    m_free.pop();
    return Handle(*this, std::move(conn));
}

void ConnectionPool::Release(std::shared_ptr<pqxx::connection> conn)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_free.push(std::move(conn));
    }
    m_cv.notify_one();
}
