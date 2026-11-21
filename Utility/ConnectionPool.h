#ifndef ConnectionPool_H
#define ConnectionPool_H

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>

// Пул соединений с БД
class ConnectionPool
{
public:
    ConnectionPool(const std::string& connStr, std::size_t size);

    // RAII-обёртка: возвращение соединения в пул при разрушении
    class Handle {
    public:
        Handle(ConnectionPool& pool, std::shared_ptr<pqxx::connection> conn)
            : m_pool(pool), m_conn(std::move(conn)) {}
        ~Handle() { m_pool.Release(m_conn); }
        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;

        pqxx::connection& Get() { return *m_conn; }

    private:
        ConnectionPool& m_pool;
        std::shared_ptr<pqxx::connection> m_conn;
    };

    Handle Get();

private:
    void Release(std::shared_ptr<pqxx::connection> conn);

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::shared_ptr<pqxx::connection>> m_free;
    std::string m_connStr;
};

#endif
