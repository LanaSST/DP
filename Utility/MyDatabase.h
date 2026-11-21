#ifndef MyDatabase_H
#define MyDatabase_H

#include <list>
#include <map>
#include <string>
#include <pqxx/pqxx>

struct SearchResult {
    int m_nDocumentID{-1};
    std::string m_strDocumentName;
    int m_nRelevance{0};
};

struct Document {
    Document() {}
    Document(const Document& src) = default;
    Document(const std::string& strName_, const std::string& strContent_)
        : strName(strName_)
        , strContent(strContent_)
    {}

    int nID{-1};
    std::string strName;
    std::string strContent;
    std::map<std::string, int> mapWordFrequencies; //{key=word -> value=freq}
};

class MyDatabase
{
public:
    explicit MyDatabase(pqxx::connection& externalConn);

    void CreateDatabase();

    // Документы
    void AddDocument(Document& document_);
    std::vector<SearchResult> SearchDocuments(const std::vector<std::string>& listWord, int limit = 10);

    ~MyDatabase() {}

private:
    // Получение id слова и создание при необходимости
    int GetOrCreateWordId(pqxx::work& tx, const std::string& strWord);

private:    
    std::shared_ptr<pqxx::connection> conn{nullptr};
    bool m_ownsConnection{true};
};

#endif
