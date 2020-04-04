#pragma once

#include <Database.hpp>
#include <exception>

#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

namespace databaseTools
{

#ifdef BUILD_TEST
class QueryTest;
#endif

class Database;

class QueryException : public std::exception
{
  public:
    QueryException(const char* what) : m_what(what)
    {

    }
    ~QueryException() override = default;

    const char* what() const noexcept override
    {
        return m_what;
    }

  private:
    const char* m_what = nullptr;
};

/**
 * @brief Abstract class for Query generation
 */
class Query
{
#ifdef BUILD_TEST
    friend class databaseTools::QueryTest;
#endif
  public:

    /**
     * @brief Types of Query available
     */
    enum QueryTypes
    {
        SELECT, ///< Select query : get values
        INSERT, ///< Insert query : add new values
        CREATE, ///< Create table query : create a new table
        UPDATE, ///< Update query : edit values
        DELETE  ///< Delete query : delete values from a table
    };

    /**
     * @brief Comparaison operators
     */
    enum Operator
    {
        EQUAL, ///< Equal
        GT, ///< Greater Than
        GE, ///< Greater or Equal
        LT, ///< Lesser than
        LE, ///< Lesser or Equal
        NOT ///< Not equal
    };

    /**
     * @brief Column constraints
     */
    enum Constraints
    {
        PRIMARY_KEY, ///< Primary key
        UNIQUE, ///< Unique
        AUTOINCREMENT, ///< Auto increment
        NOT_NULL ///< Not null
    };

    /**
     * @brief Join types
     */
    enum JoinType
    {
        INNER_JOIN, ///< 1 for 1 join
        LEFT_JOIN ///< Get all the left table result even if there is no match on the right table
    };

    /**
     * @brief Store column with the table related
     */
    class Column
    {
      public:
        /**
         * @brief Create a column without table
         * @param columnName Name of the column
         */
        Column(const char* columnName) : m_columnName(columnName) {}

        /**
         * @brief Create a column without table
         * @param columnName Name of the column
         */
        Column(const std::string& columnName) : m_columnName(columnName) {}

        /**
         * @brief Create a column linked to a table
         * @param tableName Name of the table
         * @param columnName Name of the column
         */
        Column(const char* tableName, const char* columnName) : m_tableName(tableName),
            m_columnName(columnName) {}

        /**
         * @brief Create a column linked to a table
         * @param tableName Name of the table
         * @param columnName Name of the column
         */
        Column(const std::string& tableName,
               const std::string& columnName) : m_tableName(tableName),
            m_columnName(columnName) {}

        /**
         * @brief Convert the couple table+column in SQL format with '.' notation
         */
        std::string str() const
        {
            return (!m_tableName.empty() ? m_tableName + "." : "") + m_columnName;
        }
        std::string tableName() const
        {
            return m_tableName;
        }
        void setTableName(const std::string& table)
        {
            m_tableName = table;
        }
        std::string columnName() const
        {
            return m_columnName;
        }
        void setColumnName(const std::string& column)
        {
            m_columnName = column;
        }

      private:
        std::string m_tableName = "";     ///< Name of the table, optionnal
        std::string m_columnName;         ///< Name of the column
    };

  private:
    /**
     * @struct FindQueryType
     * @brief Return the class type associated with the QueryTypes given (specified later)
     */
    template<QueryTypes T>
    struct FindQueryType {};
  public:

    template<QueryTypes T> static typename FindQueryType<T>::type createQuery(
        const std::string& table, std::shared_ptr<Database> db);
    /// @brief Construct a Query
    Query(const std::string& table, std::shared_ptr<Database> db) : m_table(table),
        m_db(db) {}
    ~Query() = default;

    /**
     * @brief Return the query in a string
     *
     * Must be overriden in derived classes
     *
     * @return String
     */
    virtual std::string str() const = 0;
    /**
     * @brief Return if the Query is valid or not. The validity is set in the derived classes
     * @return If the query is not valid, str() will return ""
     */
    virtual bool isValid() const
    {
        return m_valid;
    }
  protected:
    DataType dataType(Column column);
    std::string operatorAsString(Operator op);
    void checkColumnName(const Column& column);
    bool checkColumnNameValidity(const Column& column);
    bool checkColumnExistance(Column column);

    /**
     * @brief Add condition to the condition list
     *
     * Use with caution, there is no verification
     * @param [in,out] conditions Condition list to modify
     * @param [in] condition Condition to add
     */
    virtual void doWhere(std::vector<std::string>& conditions,
                         const std::string& condition) final
    {
        conditions.push_back(condition);
    }
    virtual void doWhere(std::vector<std::string>& conditions, const Column& column,
                         Operator op, std::string value) final;
    virtual void doColumn(std::vector<std::string>& columns,
                          const Column& column) final;
    virtual void doValue(std::vector<std::pair<std::string, std::string>>& values,
                         const Column& column, std::string value) final;
    virtual void doSort(std::vector<std::string>& sortColumns,
                        const Column& column) final;
    virtual void doJoin(const std::string& table, const std::string& localColumn,
                        const std::string& distantColumn, JoinType type = JoinType::INNER_JOIN) final;

    virtual std::stringstream joinStatement() const final;

    std::string table() const
    {
        return m_table;
    }

    void setValid(bool valid)
    {
        m_valid = valid;
    }

    std::shared_ptr<Database> db() const
    {
        return m_db;
    }

    /**
     * @brief Join informations
     */
    struct Join
    {
        std::string table;              ///< Table to join
        std::string localColumn;        ///< Local column
        std::string
        distantColumn;      ///< Distant column to join with the local column
        JoinType type;                  ///< Type of join
    };

  private:
    std::string m_table;                ///< Name of the table targeted by the Query
    std::vector<Join> m_joins;          ///< List of table joins
    std::shared_ptr<Database>
    m_db; ///< Database where the Query will apply (used for verifications)
    bool m_valid = false;               ///< Validity of the Query
};

/**
 * @brief Create a SELECT Query
 */
class SelectQuery : public Query
{
  public:
    /// @brief Construct a SELECT Query
    SelectQuery(const std::string& table,
                std::shared_ptr<Database> db) : Query(table, db)
    {
        setValid(true);
    }
    ~SelectQuery() = default;

    /// @brief Add a selected column
    SelectQuery& column(const Column& column)
    {
        doColumn(m_columns, column);
        return *this;
    }
    /// @brief Add a filter condition
    SelectQuery& where(const std::string& condition)
    {
        doWhere(m_conditions, condition);
        return *this;
    }
    /// @brief Add a filter condition
    SelectQuery& where(const Column& column, Operator op, const std::string& value)
    {
        doWhere(m_conditions, column, op, value);
        return *this;
    }
    /// @brief Add a sort column
    SelectQuery& sort(const Column& column, bool ascending = true)
    {
        doSort(m_sortColumns, column);
        m_sortAscending = ascending;
        return *this;
    }

    /// @brief Join a table
    SelectQuery& join(const std::string& table, const std::string& localColumn,
                      const std::string& distantColumn, JoinType type = JoinType::INNER_JOIN)
    {
        doJoin(table, localColumn, distantColumn, type);
        return *this;
    }

    std::string str() const override;

  private:
    std::vector<std::string> m_columns; ///< Columns selected
    std::vector<std::string> m_conditions; ///< Selection conditions
    std::vector<std::string> m_sortColumns; ///< Columns to sort
    bool m_sortAscending = true; ///< Result sorting
};

/**
 * @brief Create an INSERT Query
 */
class InsertQuery : public Query
{
  public:
    /// @brief Construct an INSERT Query
    InsertQuery(const std::string& table,
                std::shared_ptr<Database> db) : Query(table, db) {}
    ~InsertQuery() = default;

    /// @brief Add a value to the adding list
    InsertQuery& value(const Column& column, const std::string& value)
    {
        doValue(m_values, column, value);
        return *this;
    }

    std::string str() const override;
  private:
    std::vector<std::pair<std::string, std::string>> m_values; ///< Values to insert
};

/**
 * @brief Create an CREATE TABLE Query
 */
class CreateQuery : public Query
{
  public:
    /// @brief Construct a CREATE TABLE Query
    CreateQuery(const std::string& table,
                std::shared_ptr<Database> db) : Query(table, db) {}
    ~CreateQuery() = default;

    /// @brief Add "IF NOT EXISTS" statement to the create table query
    CreateQuery& ifNotExists()
    {
        m_ifNotExists = true;
        return *this;
    }
    CreateQuery& column(const std::string& columnName,
                        DataType columnType = DataType::BLOB,
                        const std::string& fkTable = "",
                        const std::string& fkField = "");
    /// @brief Add the table contraint
    CreateQuery& constraint(const std::string& columnName,
                            Query::Constraints constraintType);

    std::string str() const override;
  private:
    bool m_ifNotExists = false; ///< "IF NOT EXISTS" statement add

    /**
     * @brief Columns to create
     *
     * The tuple contains :
     * - the column name
     * - the type of the column
     * - table for primary key
     * - fields for the primary key
     */
    std::vector<std::tuple<std::string, DataType, std::string, std::string>>
            m_columns;

    std::vector<std::string>
    m_primaryKeyColumns; ///< Columns with Primary Key constraint
    std::vector<std::string> m_uniqueColumns;  ///< Columns with Unique constraint
    std::vector<std::string>
    m_autoincrementColumns;  ///< Columns with Auto increment constraint
    std::vector<std::string> m_notNullColumns; ///< Columns with Not null constraint

};

/**
 * @brief Create an UPDATE query
 */
class UpdateQuery : public Query
{
  public:
    /// @brief Construct a UPDATE Query
    UpdateQuery(const std::string& table,
                std::shared_ptr<Database> db) : Query(table, db) {}
    ~UpdateQuery() = default;

    UpdateQuery& set(const std::string& columnName, const std::string& value);
    /// @brief Add filter condition
    UpdateQuery& where(const std::string& condition)
    {
        doWhere(m_conditions, condition);
        return *this;
    }
    UpdateQuery& where(const std::string& column, Query::Operator op,
                       const std::string& value)
    {
        doWhere(m_conditions, Column{column}, op, value);
        return *this;
    }

    std::string str() const override;
  private:
    std::map<std::string, std::string> m_set; ///< Couples column name / new value
    std::vector<std::string> m_conditions; ///< Filter for update

};


/**
 * @brief Create an DELETE query
 * @warning If no "where" is given, this query delete all the row from the table
 */
class DeleteQuery : public Query
{
  public:
    /// @brief Construct a UPDATE Query
    DeleteQuery(const std::string& table,
                std::shared_ptr<Database> db) : Query(table, db)
    {
        setValid(true);
    }
    ~DeleteQuery() = default;

    /// @brief Add filter condition
    DeleteQuery& where(const std::string& condition)
    {
        doWhere(m_conditions, condition);
        return *this;
    }
    /// @brief Add filter condition
    DeleteQuery& where(const std::string& column, Query::Operator op,
                       const std::string& value)
    {
        doWhere(m_conditions, Column{column}, op, value);
        return *this;
    }

    std::string str() const override;
  private:
    std::vector<std::string> m_conditions; ///< Filter for update

};

/// QueryTypes::SELECT into SelectQuery
template<> struct Query::FindQueryType<Query::SELECT>
{
    typedef SelectQuery type; /**< Type matching QueryTypes::SELECT */
};
/// QueryTypes::INSERT into InsertQuery
template<> struct Query::FindQueryType<Query::INSERT>
{
    typedef InsertQuery type; /**< Type matching QueryTypes::INSERT */
};
/// QueryTypes::CREATE into CreateQuery
template<> struct Query::FindQueryType<Query::CREATE>
{
    typedef CreateQuery type; /**< Type matching QueryTypes::CREATE */
};
/// QueryTypes::UPDATE into UpdateQuery
template<> struct Query::FindQueryType<Query::UPDATE>
{
    typedef UpdateQuery type; /**< Type matching QueryTypes::UPDATE */
};
/// QueryTypes::DELETE into DeleteQuery
template<> struct Query::FindQueryType<Query::DELETE>
{
    typedef DeleteQuery type; /**< Type matching QueryTypes::UPDATE */
};

/**
 * @fn Query::FindQueryType<T>::type Query::createQuery(const std::string& table)
 * @brief Create the QUery which match the QueryType given
 * @param table Name of the table on which the query will be applied
 */
template<Query::QueryTypes T>
typename Query::FindQueryType<T>::type Query::createQuery(
    const std::string& table, std::shared_ptr<Database> db)
{
    return typename Query::FindQueryType<T>::type(table, db);
}

} // namespace database

