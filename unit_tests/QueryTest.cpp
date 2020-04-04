#include "QueryTest.hpp"
#include "Database.hpp"

#ifdef BUILD_USE_FILESYSTEM
#include <filesystem>
#endif
namespace databaseTools
{

/*
 * Test SELECT Query generation
 */
TEST_F(QueryTest, Select)
{
    std::string expected = "SELECT id, name FROM table1 WHERE id = 3;";

    auto query = Query::createQuery<Query::SELECT>("table1",
                 m_database).column("id").column("name").where("id = 3");

    EXPECT_TRUE(dynamic_cast<SelectQuery*>(&query));
    EXPECT_EQ(query.str(), expected);

    EXPECT_EQ(m_database->columnsType("table1").size(), 2);

    expected = "SELECT id, name FROM table1 WHERE id = 2;";
    query = Query::createQuery<Query::SELECT>("table1",
            m_database).column("id").column("name").where("id", Query::EQUAL, "2");

    EXPECT_EQ(query.str(), expected);

    expected = "SELECT id, name FROM table1 WHERE name = 'Jack';";
    query = Query::createQuery<Query::SELECT>("table1",
            m_database).column("id").column("name").where("name", Query::EQUAL, "Jack");

    EXPECT_EQ(query.str(), expected);

    expected =
        "SELECT id, name FROM table1 WHERE name = 'Jack' ORDER BY name DESC;";
    query = Query::createQuery<Query::SELECT>("table1",
            m_database).column("id").column("name").where("name", Query::EQUAL, "Jack")
            .sort("name", false);
    EXPECT_EQ(query.str(), expected);
}

/*
 * Test INSERT Query generation
 */
TEST_F(QueryTest, Insert)
{
    std::string expected = "";
    auto query = Query::createQuery<Query::INSERT>("table2", m_database);
    EXPECT_TRUE(dynamic_cast<InsertQuery*>(&query));
    EXPECT_EQ(query.str(), expected);
    EXPECT_FALSE(query.isValid());

    expected =
        "INSERT INTO table2 (first_name, last_name) VALUES ('Solomon', 'Lane');";
    EXPECT_EQ(Query::createQuery<Query::INSERT>("table2", m_database)
              .value("first_name", "Solomon")
              .value("last_name", "Lane").str(), expected);
}

/*
 * Test CREATE Query generation
 */
TEST_F(QueryTest, Create)
{
    std::string expected = "";
    auto query = Query::createQuery<Query::CREATE>("newTable", m_database);
    EXPECT_TRUE(dynamic_cast<CreateQuery*>(&query));
    EXPECT_EQ(query.str(), expected);
    EXPECT_FALSE(query.isValid());

    query = Query::createQuery<Query::CREATE>("newTable",
            m_database).constraint("first_name", Query::PRIMARY_KEY)
            .constraint("last_name", Query::PRIMARY_KEY);
    EXPECT_EQ(query.str(), expected);
    EXPECT_FALSE(query.isValid());

    expected =
        "CREATE TABLE newTable (first_name BLOB, last_name BLOB, age INTEGER, PRIMARY KEY(`first_name`, `last_name`));";
    query = Query::createQuery<Query::CREATE>("newTable", m_database)
            .column("first_name").column("last_name").column("age", INTEGER)
            .constraint("first_name", Query::PRIMARY_KEY)
            .constraint("last_name", Query::PRIMARY_KEY);

    EXPECT_EQ(query.str(), expected);
    EXPECT_TRUE(query.isValid());

    expected =
        "CREATE TABLE newTable (first_name BLOB UNIQUE, last_name BLOB NOT NULL, age INTEGER AUTOINCREMENT);";
    query = Query::createQuery<Query::CREATE>("newTable", m_database)
            .column("first_name").column("last_name").column("age", INTEGER)
            .constraint("first_name", Query::UNIQUE)
            .constraint("last_name", Query::NOT_NULL)
            .constraint("age", Query::AUTOINCREMENT);

    EXPECT_EQ(query.str(), expected);
    EXPECT_TRUE(query.isValid());

    expected =
        "CREATE TABLE newTable (first_name BLOB, last_name BLOB, age INTEGER);";
    query = Query::createQuery<Query::CREATE>("newTable", m_database)
            .column("first_name").column("last_name").column("age", INTEGER)
            .constraint("not_a_column", Query::UNIQUE);

    EXPECT_EQ(query.str(), expected);
    EXPECT_TRUE(query.isValid());
}

TEST_F(QueryTest, CreateWithForeignKey)
{
#ifdef BUILD_USE_FILESYSTEM
    std::filesystem::path usedFile = "data/sample0.db";
    std::filesystem::remove(usedFile);

    std::shared_ptr<databaseTools::Database> db(new databaseTools::Database(
                usedFile));

    db->query(Query::createQuery<Query::CREATE>("table1", db)
              .column("column1", DataType::BLOB));

    std::string expected =
        "CREATE TABLE newTable (first_name BLOB REFERENCES table1(`column1`));";
    auto query = Query::createQuery<Query::CREATE>("newTable", db)
                 .column("first_name", DataType::BLOB, "table1", "column1");

    EXPECT_EQ(query.str(), expected);
    EXPECT_TRUE(query.isValid());
#endif
}

/*
 * Test UPDATE Query generation
 */
TEST_F(QueryTest, Update)
{
    std::string expected = "";
    auto query = Query::createQuery<Query::UPDATE>("table3", m_database);

    EXPECT_TRUE(dynamic_cast<UpdateQuery*>(&query));
    EXPECT_EQ(query.str(), expected);
    EXPECT_FALSE(query.isValid());

    query = Query::createQuery<Query::UPDATE>("table3",
            m_database).where("i = 1");
    EXPECT_EQ(query.str(), expected);
    EXPECT_FALSE(query.isValid());

    expected = "UPDATE table3 SET i = 1, j = 3 WHERE k = 3;";
    query = Query::createQuery<Query::UPDATE>("table3", m_database)
            .set("i", "1").set("j", "3")
            .where("k = 3");
    EXPECT_EQ(query.str(), expected);
    EXPECT_TRUE(query.isValid());
}

TEST_F(QueryTest, Delete)
{
    std::string expected = "DELETE FROM table1 WHERE field1 = 'Bob';";
    auto query = Query::createQuery<Query::DELETE>("table1",
                 m_database).where("field1", Query::EQUAL, "Bob");
    EXPECT_EQ(query.str(), expected);
}

TEST_F(QueryTest, InnerJoin)
{
    std::string expected =
        "SELECT * FROM table1 INNER JOIN table2 ON table1.field1 = table2.field2;";
    auto query = Query::createQuery<Query::SELECT>("table1",
                 m_database).join("table2", "field1", "field2");
    EXPECT_EQ(query.str(), expected);

    expected =
        "SELECT * FROM table1 INNER JOIN table2 ON table1.field1 = table2.field2 WHERE field1 = 'Bob';";
    query = Query::createQuery<Query::SELECT>("table1",
            m_database).join("table2",
                             "field1", "field2").where("field1", Query::EQUAL, "Bob");
    EXPECT_EQ(query.str(), expected);
}

void QueryTest::SetUp()
{
    m_database.reset(new Database("data/db_sample2.db"));
}

}


