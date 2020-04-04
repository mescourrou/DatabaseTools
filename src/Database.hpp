#pragma once

// STD lib
#include <mutex>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

// Libs
#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

class sqlite3;


/**
 * @namespace database
 * @brief Group all classes usefull for database use
 */
namespace databaseTools
{

class DatabaseException : public std::exception
{
  public:
    DatabaseException(const char* what) : m_what(what)
    {

    }
    ~DatabaseException() override = default;

    const char* what() const noexcept override
    {
        return m_what;
    }

  private:
    const char* m_what = nullptr;
};

#ifdef BUILD_TEST
class DatabaseTest;
#endif
class Query;

/**
 * @brief Possibles types of the data
 */
enum DataType : int
{
    INTEGER,
    TEXT,
    REAL,
    NUMERIC,
    BLOB
};

/**
 * @brief Class for Database manipulation
 */
class Database
{
#ifdef BUILD_TEST
    friend class databaseTools::DatabaseTest;
    FRIEND_TEST(DatabaseTest, QueryText);
#endif
  public:
    explicit Database(const std::string& path);
    ~Database();

    std::vector<std::map<std::string, std::string>> query(const Query& dbQuery);
    bool query(const std::string& query);
    static bool isQuerySuccessfull(const
                                   std::vector<std::map<std::string, std::string>>& result);

    /**
     * @brief Return a lockguard of the database mutex
     *
     * It is usefull when you want to use custom string queries
     */
    std::lock_guard<std::mutex> lockGuard()
    {
        return std::lock_guard<std::mutex>(m_queryMutex);
    }

    std::vector<std::string> tableList();
    bool isTable(const std::string& table);
    std::vector<std::string> columnList(const std::string& table);
    std::map<std::string, DataType> columnsType(const std::string& table);

    static std::string dataTypeAsString(const DataType& data);
    static DataType dataTypeFromString(const std::string& data);

    static void setVerbosity(bool verbosity = true);
  private:
    int callback(int argc, char** argv, char** colName);

    static inline bool m_verbosity = false;
    std::mutex m_queryMutex;    ///< Mutex to protect the result access

    sqlite3* m_sqlite3Handler = nullptr; ///< Pointer on sqlite handler
    std::unique_ptr<std::vector<std::map<std::string, std::string>>>
    m_result; ///< Saving results temporary

};


} // namespace database

