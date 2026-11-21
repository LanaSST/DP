#include <iostream>
#include <exception>
#include <algorithm>
#include <syncstream>
#include "MyDatabase.h"

MyDatabase::MyDatabase(pqxx::connection& externalConn)
    : conn(&externalConn, [](pqxx::connection*){})
    , m_ownsConnection(false)
{
    if (!conn->is_open())
        throw std::runtime_error("DB is not connected");
}

void MyDatabase::CreateDatabase() {
    pqxx::work ts{*conn};

    ts.exec(R"(
        CREATE TABLE IF NOT EXISTS documents (
            id SERIAL PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            content TEXT
        )
    )");

    ts.exec(R"(
        CREATE TABLE IF NOT EXISTS words (
            id SERIAL PRIMARY KEY,
            word VARCHAR(255) NOT NULL UNIQUE
        )
    )");

    ts.exec(R"(
        CREATE TABLE IF NOT EXISTS word_frequencies (
            id SERIAL PRIMARY KEY,
            document_id INTEGER NOT NULL REFERENCES documents(id) ON DELETE CASCADE,
            word_id INTEGER NOT NULL REFERENCES words(id) ON DELETE CASCADE,
            frequency INTEGER NOT NULL DEFAULT 0 CHECK (frequency >= 0),
            UNIQUE (document_id, word_id)
        )
    )");

    ts.commit();
    std::cout << "Tables created successfully!" << std::endl;
}

int MyDatabase::GetOrCreateWordId(pqxx::work& tx, const std::string& strWord) {
    // Пробуем вставить; при конфликте — просто читаем существующий id
    pqxx::result res = tx.exec(
        "INSERT INTO words (word) VALUES ($1) "
        "ON CONFLICT (word) DO UPDATE SET word = EXCLUDED.word "
        "RETURNING id",
        pqxx::params{strWord}
    );
    return res[0][0].as<int>();
}

void MyDatabase::AddDocument(Document& document_) {
    try {
        pqxx::work tx(*conn);

        pqxx::result res = tx.exec(
            "INSERT INTO documents (name, content) VALUES ($1, $2) RETURNING id",
            pqxx::params{document_.strName, document_.strContent}
        );
        document_.nID = res[0][0].as<int>();

        for (const auto& [word, freq] : document_.mapWordFrequencies) {
            if (freq <= 0) continue;
            int wordId = GetOrCreateWordId(tx, word);
            tx.exec(
                "INSERT INTO word_frequencies (document_id, word_id, frequency) "
                "VALUES ($1, $2, $3) "
                "ON CONFLICT (document_id, word_id) DO UPDATE "
                "SET frequency = EXCLUDED.frequency",
                pqxx::params{document_.nID, wordId, freq}
            );
        }

        tx.commit();
        std::osyncstream(std::cout) << "Document added! ID: " << document_.nID  << " Name: " << document_.strName << std::endl;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Document not added: " + std::string(e.what()));
    }
}

std::vector<SearchResult> MyDatabase::SearchDocuments(const std::vector<std::string>& listWord, int limit)
{
    std::vector<SearchResult> results;
    if (listWord.empty()) return results;

    try {
        pqxx::work tx(*conn);

        std::string params;
        for (std::size_t i = 0; i < listWord.size(); ++i) {
            if (i) params += ", ";
            params += "$" + std::to_string(i + 1);
        }

        // SQL:
        //  - join word_frequencies + listWord, выбираем слова из запроса
        //  - group by документ, сумма частот
        //  - having count(distinct word_id) = N — документ содержит ВСЕ слова
        //  - order by sum(frequency) desc
        //  - limit
        std::string sql =
            "SELECT d.id, d.name, SUM(wf.frequency) AS relevance "
            "FROM documents d "
            "JOIN word_frequencies wf ON wf.document_id = d.id "
            "JOIN words w ON w.id = wf.word_id "
            "WHERE w.word IN (" + params + ") "
            "GROUP BY d.id, d.name "
            "HAVING COUNT(DISTINCT w.id) = $" + std::to_string(listWord.size() + 1) + " "
            "ORDER BY relevance DESC "
            "LIMIT $" + std::to_string(listWord.size() + 2);

        pqxx::params p;
        for (const auto& w : listWord) p.append(w);
        p.append(static_cast<int>(listWord.size()));   // HAVING
        p.append(limit);                            // LIMIT

        pqxx::result res = tx.exec(sql, p);
        tx.commit();

        for (const auto& row : res) {
            SearchResult r;
            r.m_nDocumentID   = row[0].as<int>();
            r.m_strDocumentName = row[1].as<std::string>();
            r.m_nRelevance    = row[2].as<int>();
            results.push_back(std::move(r));
        }
    }
    catch (const std::exception& e) {
        std::cout << "SearchDocuments error: " << e.what() << std::endl;
        throw std::runtime_error("SearchDocuments error: " + std::string(e.what()));
    }
    return results;
}

