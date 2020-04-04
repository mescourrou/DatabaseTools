#pragma once

#include <gtest/gtest.h>
#include <Query.hpp>

namespace databaseTools
{

class QueryTest : public testing::Test
{
  public:
    void SetUp() override;

    std::shared_ptr<Database> m_database;
};

}

