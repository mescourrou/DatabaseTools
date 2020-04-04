#include "DatabaseTest.hpp"
#include "Query.hpp"
#include <glog/logging.h>

#ifdef BUILD_USE_FILESYSTEM
#include <filesystem>
#endif
namespace databaseTools
{

#ifdef BUILD_USE_FILESYSTEM
/*
 * Test query execution and good return
 */
TEST_F(DatabaseTest, QueryText)
{
    auto lock = m_database->lockGuard();
    EXPECT_TRUE(m_database->query("SELECT * FROM table1 WHERE id = 1"));
    ASSERT_EQ(m_database->m_result->size(), 3);

    EXPECT_EQ(m_database->m_result->at(0).at("status"), "success");

    EXPECT_EQ(m_database->m_result->at(1).at("id"), "1");
    EXPECT_EQ(m_database->m_result->at(1).at("name"), "Good morning");

    EXPECT_EQ(m_database->m_result->at(2).at("id"), "1");
    EXPECT_EQ(m_database->m_result->at(2).at("name"), "Good evening");

}

/*
 * Test SELECT Query
 */
TEST_F(DatabaseTest, QuerySelect)
{
    auto result = m_database->query(Query::createQuery<Query::SELECT>("table1",
                                    m_database)
                                    .where("id = 2"));

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).at("status"), "success");

    EXPECT_EQ(result.at(1).at("id"), "2");
    EXPECT_EQ(result.at(1).at("name"), "Hello World");

    result = m_database->query(Query::createQuery<Query::SELECT>("table1",
                               m_database)
                               .where("id = 1").where("name = 'Good morning'"));

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).at("status"), "success");

    EXPECT_EQ(result.at(1).at("id"), "1");
    EXPECT_EQ(result.at(1).at("name"), "Good morning");

    // No result
    result = m_database->query(Query::createQuery<Query::SELECT>("table1",
                               m_database)
                               .where("id = 1").where("name = 'I dont exist'"));

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.at(0).at("status"), "success");
}

/*
 * INSERT Query test
 */
TEST_F(DatabaseTest, Insert)
{
    // Execute INSERT Query
    auto result = m_database->query(Query::createQuery<Query::INSERT>("table1",
                                    m_database)
                                    .value("id", "3").value("name", "Hello guys"));
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.at(0).at("status"), "success");

    // Verify with a select Query
    result = m_database->query(Query::createQuery<Query::SELECT>("table1",
                               m_database)
                               .where("id = 3"));
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).at("status"), "success");
    EXPECT_EQ(result.at(1).at("id"), "3");
    EXPECT_EQ(result.at(1).at("name"), "Hello guys");
}

/*
 * UPDATE Query
 */
TEST_F(DatabaseTest, Update)
{
    // Execute UPDATE Query
    auto result = m_database->query(Query::createQuery<Query::UPDATE>("table1",
                                    m_database)
                                    .set("id", "4").where("name = 'Good evening'"));
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.at(0).at("status"), "success");

    // Verify with a select Query
    result = m_database->query(Query::createQuery<Query::SELECT>("table1",
                               m_database)
                               .where("id = 4"));
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).at("status"), "success");
    EXPECT_EQ(result.at(1).at("id"), "4");
    EXPECT_EQ(result.at(1).at("name"), "Good evening");
}

/*
 * Test getting the table list
 */
TEST_F(DatabaseTest, TableList)
{
    auto tableList = m_database->tableList();
    EXPECT_EQ(tableList.size(), 2);
    EXPECT_TRUE(std::find(tableList.begin(), tableList.end(),
                          "table1") != tableList.end());
    EXPECT_TRUE(std::find(tableList.begin(), tableList.end(),
                          "table2") != tableList.end());
    EXPECT_FALSE(std::find(tableList.begin(), tableList.end(),
                           "noExistente") != tableList.end());
}

/*
 * Get the list of the columns
 */
TEST_F(DatabaseTest, TableColumnList)
{
    auto columnList = m_database->columnList("table1");
    EXPECT_EQ(columnList.size(), 2);
    EXPECT_TRUE(std::find(columnList.begin(), columnList.end(),
                          "id") != columnList.end());
    EXPECT_TRUE(std::find(columnList.begin(), columnList.end(),
                          "name") != columnList.end());
    EXPECT_FALSE(std::find(columnList.begin(), columnList.end(),
                           "notAColumn") != columnList.end());
}

/*
 * Get the list of column types
 */
TEST_F(DatabaseTest, TableTypeList)
{
    auto list = m_database->columnsType("table1");
    EXPECT_EQ(list.size(), 2);
    ASSERT_TRUE(list.count("id"));
    EXPECT_EQ(list["id"], INTEGER);

    ASSERT_TRUE(list.count("name"));
    EXPECT_EQ(list["name"], TEXT);

    EXPECT_FALSE(list.count("notAColumn"));
}

TEST_F(DatabaseTest, InnerJoin)
{
    auto result = m_database->query(Query::createQuery<Query::SELECT>("table2",
                                    m_database)
                                    .join("table1", "uniqueID", "id")
                                    .where("name", Query::EQUAL, "Hello World")
                                    .column("City"));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(1).at("City"), "Chicago");
}

/**
 * @brief Set up the tests
 */
void DatabaseTest::SetUp()
{
    std::filesystem::path usedFile = "data/db_sample1.db";
    std::filesystem::path modelFile = "data/db_sample1.db.in";
    // Remove the file of previous tests
    if (std::filesystem::exists(usedFile))
        std::filesystem::remove(usedFile);
    if (!std::filesystem::exists(modelFile))
        LOG(FATAL) << "File " << modelFile << " doen't exists";
    // Copy original file
    std::filesystem::copy(modelFile, usedFile);
    m_database.reset(new Database(usedFile));
}

#endif
}


int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

