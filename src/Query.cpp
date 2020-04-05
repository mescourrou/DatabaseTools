#include "Query.hpp"

#include <algorithm>

// External libs
#include <glog/logging.h>

namespace databaseTools
{

/**
 * @brief Get the datatype of a column
 * @param [in] column Column to look for
 * @return database::DataType matching
 */
DataType Query::dataType(Column column)
{
    if (!db())
        throw QueryException("No database given");
    if (column.tableName().empty())
        column.setTableName(table());
    auto types = db()->columnsType(column.tableName());
    if (types.find(column.columnName()) == types.end())
    {
        LOG(WARNING) << "Requested column (" << column.columnName() <<
                     ") not found in the table " << column.tableName();
        return BLOB;
    }

    return types.at(column.columnName());
}

/**
 * @brief Convert the operator given in string
 * @param [in] op Operator to convert
 * @return std::string corresponding to the operator
 */
std::string Query::operatorAsString(Query::Operator op)
{
    switch (op)
    {
    case EQUAL:
        return "=";
    case GT:
        return ">";
    case GE:
        return ">=";
    case LT:
        return "<";
    case LE:
        return "<=";
    case NOT:
        return "NOT";
    default:
        return "";
    }
}

/**
 * @brief Check if the column name is valid and if the column exists
 * @param [in] name Name of the column to check
 */
void Query::checkColumnName(const Column& column)
{
    if (!checkColumnNameValidity(column))
        throw QueryException(std::string("Column name not valid : ").append(
                                 column.tableName()).c_str());
    if (!checkColumnExistance(column))
        throw QueryException(
            std::string("Column name not existent in the database : ").append(
                column.tableName()).c_str());

}

/**
 * @brief Verify if the name is valid
 *
 * Check if the name does not have space.
 *
 * @param [in] name Name to check
 * @return Return true if the name is valid
 */
bool Query::checkColumnNameValidity(const Column& column)
{
    if (column.columnName().find(' ') != std::string::npos)
    {
        setValid(false);
        LOG(ERROR) << "Not valid column name : '" << column.columnName() << "'";
        return false;
    }
    return true;
}

/**
 * @brief Verify if the column exists
 *
 * If the column doesn't exist, set the validity of the Query to false.
 * @param [in] name Name of the column to check
 * @return Return true if the column exists
 */
bool Query::checkColumnExistance(Column column)
{
    if (column.tableName().empty())
        column.setTableName(table());
    auto columnList = db()->columnList(column.tableName());
    if (std::find(columnList.begin(), columnList.end(),
                  column.columnName()) == columnList.end())
    {
        setValid(false);
        LOG(ERROR) << "'" << column.columnName() <<
                   "' column doesn't exists in the table '" << column.tableName() << "'";
        return false;
    }
    return true;
}

/**
 * @brief Implementation of the adding of conditions to query
 * @param [in,out] conditions Condition vector to modify
 * @param [in] column Column where apply the filter
 * @param [in] op Operator of comparison
 * @param [in] value Value to compare to
 */
void Query::doWhere(std::vector<std::string>& conditions, const Column& column,
                    Query::Operator op, std::string value)
{
    auto type = dataType(column);
    if (type == BLOB || type == TEXT)
        value = std::string("'").append(value).append("'");
    conditions.push_back(std::string().append(column.str()).append(" ")
                         .append(operatorAsString(op)).append(" ").append(value));
}

/**
 * @brief Implementation of the add of column in the selection field list
 * @param [in,out] columns Column list to modify
 * @param [in] column Column to add
 */
void Query::doColumn(std::vector<std::string>& columns, const Column& column)
{
    checkColumnName(column);
    columns.push_back(column.str());
}

/**
 * @brief Add the couple <column, value> to the query
 * @param [in] column Column of the value
 * @param [in] value Value to insert
 * @return New query
 */
void Query::doValue(std::vector<std::pair<std::string, std::string>>& values,
                    const Column& column, std::string value)
{
    checkColumnName(column);
    auto type = dataType(column);
    if (type == BLOB || type == TEXT)
        value = std::string("'").append(value).append("'");
    setValid(true);
    values.push_back(std::pair<std::string, std::string>(column.str(), value));
}

/**
 * @brief Add a column to sort
 * @param sortColumns Vector of columns to sort
 * @param column Column to sort
 */
void Query::doSort(std::vector<std::string>& sortColumns, const Column& column)
{
    checkColumnName(column);
    sortColumns.push_back(column.str());
}

/**
 * @brief Add a table to join
 * @param table Table to join
 * @param localColumn Column on the local table
 * @param distantColumn Column on the new table
 * @param type Type of join type
 */
void Query::doJoin(const std::string& table, const std::string& localColumn,
                   const std::string& distantColumn, JoinType type)
{
    if (!db()->isTable(table) || !checkColumnExistance(Column(localColumn))
            || !checkColumnExistance(Column{table, distantColumn}))
        setValid(false);

    for (const auto& j : m_joins)
    {
        if (j.table == table)
        {
            setValid(false);
            return;
        }
    }

    m_joins.push_back(Join{table, localColumn, distantColumn, type});
}

/**
 * @brief Create the join statement
 */
std::stringstream Query::joinStatement() const
{
    std::stringstream ss;
    for (auto& j : m_joins)
    {
        ss << " ";
        switch (j.type)
        {
        case INNER_JOIN:
            ss << "INNER JOIN ";
            break;
        case LEFT_JOIN:
            ss << "LEFT JOIN ";
            break;
        default:
            ss << "INNER JOIN ";
            break;
        }
        ss << j.table << " ON " << table() << "." << j.localColumn << " = " << j.table
           << "." << j.distantColumn;
    }
    return ss;

}

/**
 * @brief Generate the string corresponding of the Query
 * @return std::string corresponding to the Query
 */
std::string SelectQuery::str() const
{
    std::stringstream ss;
    ss << "SELECT ";
    if (m_columns.size() == 0)
        ss << "*";
    else
    {
        for (unsigned int i = 0; i < m_columns.size(); i++)
        {
            ss << m_columns.at(i);
            if (i + 1 < m_columns.size())
                ss << ", ";
        }
    }
    ss << " FROM " << table();
    ss << joinStatement().str();
    if (m_conditions.size() != 0)
    {
        ss << " WHERE ";
        for (unsigned int i = 0; i < m_conditions.size(); i++)
        {
            ss << m_conditions.at(i);
            if (i + 1 < m_conditions.size())
                ss << " AND ";
        }
    }
    if (m_sortColumns.size() != 0)
    {
        ss << " ORDER BY ";
        for (auto& column : m_sortColumns)
        {
            ss << column;
            if (column != m_sortColumns.back())
                ss << ", ";
        }
        if (m_sortAscending)
            ss << " ASC";
        else
            ss << " DESC";
    }
    ss << ";";
    return ss.str();
}

/**
 * @brief Generate the string corresponding of the Query
 * @return std::string corresponding to the Query
 */
std::string InsertQuery::str() const
{
    if (!isValid())
        return {};
    std::stringstream ss;
    ss << "INSERT INTO ";
    ss << table();

    ss << " (";
    for (auto value : m_values)
    {
        ss << value.first;
        if (value != m_values.back())
            ss << ", ";
    }
    ss << ") VALUES (";
    for (auto value : m_values)
    {
        ss << value.second;
        if (value != m_values.back())
            ss << ", ";
    }
    ss << ")";
    ss << ";";
    return ss.str();

}

/**
 * @brief Add a column to the Creation Query
 * @param [in] columnName Name of the new column
 * @param [in] columnType Column type
 * @param [in] fkTable (optional) Table of the foreign key
 * @param [in] fkField (optional) Field of the foreign key
 * @return New Query
 */
CreateQuery& CreateQuery::column(const std::string& columnName,
                                 DataType columnType,
                                 const std::string& fkTable, const std::string& fkField)
{
    if (!checkColumnNameValidity(Column(columnName)))
        return *this;
    if (!fkTable.empty())
    {
        if (fkField.empty())
        {
            setValid(false);
            return *this;
        }
        if (!db()->isTable(fkTable))
        {
            setValid(false);
            return *this;
        }

        auto columnList = db()->columnList(fkTable);
        if (std::find(columnList.begin(), columnList.end(),
                      fkField) == columnList.end())
        {
            setValid(false);
            return *this;
        }
    }
    setValid(true);
    m_columns.push_back(std::tuple<std::string, DataType, std::string, std::string>
    {
        columnName, columnType, fkTable, fkField
    });
    return *this;
}

/**
 * @brief Add column constraint like a primary key, an autoincrement, ...
 *
 * See Query::Constraints
 * @param[in] columnName Name of the column
 * @param[in] constraintType Constraint type
 * @return Return the new Query
 */
CreateQuery& CreateQuery::constraint(const std::string& columnName,
                                     Query::Constraints constraintType)
{
    if (!checkColumnNameValidity(Column(columnName)))
        return *this;

    if (std::find_if(m_columns.begin(), m_columns.end(),
                     [&columnName](std::tuple<std::string, DataType, std::string, std::string>& a)
{
    if (std::get<0>(a) == columnName)
            return true;
        return false;
    }
                    ) == m_columns.end())
    {
        return *this;
    }

    auto addIfNotFind = [](std::vector<std::string>& list,
                           const std::string & item)
    {
        if (list.size() == 0 || std::find(list.begin(), list.end(), item) == list.end())
            list.push_back(item);
    };
    switch (constraintType)
    {
    case Query::Constraints::PRIMARY_KEY:
        addIfNotFind(m_primaryKeyColumns, columnName);
        break;
    case Query::Constraints::UNIQUE:
        addIfNotFind(m_uniqueColumns, columnName);
        break;
    case Query::Constraints::NOT_NULL:
        addIfNotFind(m_notNullColumns, columnName);
        break;
    case Query::Constraints::AUTOINCREMENT:
        addIfNotFind(m_autoincrementColumns, columnName);
        break;
    default:
        LOG(WARNING) << "Constraint type not supported (" << constraintType << ")";
        break;
    }

    return *this;
}

/**
 * @brief Generate the string corresponding of the Query
 * @return std::string corresponding to the Query
 */
std::string CreateQuery::str() const
{
    if (!isValid())
        return {};
    std::stringstream ss;
    ss << "CREATE TABLE ";
    if (m_ifNotExists)
        ss << "IF NOT EXISTS ";
    ss << table();
    ss << " (";
    for (auto& column : m_columns)
    {
        ss << std::get<0>(column);
        ss << " " << Database::dataTypeAsString(std::get<1>(column));
        if (std::find(m_notNullColumns.begin(), m_notNullColumns.end(),
                      std::get<0>(column)) != m_notNullColumns.end())
            ss << " NOT NULL";
        if (m_primaryKeyColumns.size() == 1 &&
                std::find(m_primaryKeyColumns.begin(), m_primaryKeyColumns.end(),
                          std::get<0>(column)) != m_primaryKeyColumns.end())
        {
            ss << " PRIMARY KEY";
        }
        if (std::find(m_autoincrementColumns.begin(), m_autoincrementColumns.end(),
                      std::get<0>(column)) != m_autoincrementColumns.end())
            ss << " AUTOINCREMENT";
        if (std::find(m_uniqueColumns.begin(), m_uniqueColumns.end(),
                      std::get<0>(column)) != m_uniqueColumns.end())
            ss << " UNIQUE";
        if (!std::get<2>(column).empty())
        {
            ss << " REFERENCES " << std::get<2>(column) << "(`" << std::get<3>
               (column) << "`)";
        }
        if (column != m_columns.back())
            ss << ", ";
    }
    if (m_primaryKeyColumns.size() > 1)
    {
        ss << ", PRIMARY KEY(";
        for (auto& column : m_primaryKeyColumns)
        {
            ss << "`" << column << "`";
            if (column != m_primaryKeyColumns.back())
                ss << ", ";
        }
        ss << ")";
    }
    ss << ")";
    ss << ";";
    return ss.str();
}

/**
 * @brief Add set couple
 * @param columnName Name of the column to modify
 * @param value New value
 * @return Updated Query
 */
UpdateQuery& UpdateQuery::set(const std::string& columnName,
                              const std::string& value)
{
    checkColumnName(Column(columnName));
    setValid(true);
    m_set[columnName] = value;

    return *this;
}

/**
 * @brief Generate the string corresponding of the Query
 * @return std::string corresponding to the Query
 */
std::string UpdateQuery::str() const
{
    if (!isValid())
        return {};
    std::stringstream ss;
    ss << "UPDATE " << table() << " SET ";

    unsigned int i = 0;
    auto columnTypes = db()->columnsType(table());
    for (auto value : m_set)
    {
        ss << value.first << " = ";
        DataType columnType = columnTypes.at(value.first);
        if (columnType == DataType::BLOB || columnType == DataType::TEXT)
            ss << "'" << value.second << "'";
        else
            ss << value.second;


        i++;
        if (i < m_set.size())
            ss << ", ";

    }

    if (!m_conditions.empty())
    {
        ss << " WHERE ";
        for (auto condition : m_conditions)
        {
            ss << condition;
            if (condition != m_conditions.back())
                ss << " AND ";
        }
    }

    ss << ";";
    return ss.str();
}

/**
 * @brief Generate the string corresponding of the Query
 * @return std::string corresponding to the Query
 */
std::string DeleteQuery::str() const
{
    if (!isValid())
        return {};
    std::stringstream ss;
    ss << "DELETE FROM " << table();

    if (!m_conditions.empty())
    {
        ss << " WHERE ";
        for (auto condition : m_conditions)
        {
            ss << condition;
            if (condition != m_conditions.back())
                ss << " AND ";
        }
    }

    ss << ";";
    return ss.str();
}

} // namespace database
