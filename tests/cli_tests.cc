#include <gtest/gtest.h>
#include "cli/cli.h"

class Array_value_test : public ::testing::Test {
protected:
  void SetUp() override {}
};

TEST_F(Array_value_test, ParseIntArray) {
  auto result = cli::Array_value<int>::parse("1,2,3,4,5");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 5);
  EXPECT_EQ(values[0], 1);
  EXPECT_EQ(values[4], 5);
}

TEST_F(Array_value_test, ParseStringArray) {
  auto result = cli::Array_value<std::string>::parse("hello, world, test");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 3);
  EXPECT_EQ(values[0], "hello");
  EXPECT_EQ(values[1], "world");
  EXPECT_EQ(values[2], "test");
}

TEST_F(Array_value_test, ParseBoolArray) {
  auto result = cli::Array_value<bool>::parse("true,false,1,0");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 4);
  EXPECT_TRUE(values[0]);
  EXPECT_FALSE(values[1]);
  EXPECT_TRUE(values[2]);
  EXPECT_FALSE(values[3]);
}

TEST_F(Array_value_test, ParseDoubleArray) {
  auto result = cli::Array_value<double>::parse("1.1,2.2,3.3");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 3);
  EXPECT_DOUBLE_EQ(values[0], 1.1);
  EXPECT_DOUBLE_EQ(values[2], 3.3);
}

TEST_F(Array_value_test, ToString) {
  cli::Array_value<int> arr({1, 2, 3});
  EXPECT_EQ(arr.to_string(), "1,2,3");

  cli::Array_value<std::string> str_arr({"a", "b", "c"});
  EXPECT_EQ(str_arr.to_string(), "a,b,c");
}

class Map_value_test : public ::testing::Test {
protected:
  void SetUp() override {}
};

TEST_F(Map_value_test, ParseStringMap) {
  auto result = cli::Map_value<std::string, std::string>::parse("key1=value1,key2=value2");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 2);
  EXPECT_EQ(values.at("key1"), "value1");
  EXPECT_EQ(values.at("key2"), "value2");
}

TEST_F(Map_value_test, ParseIntMap) {
  auto result = cli::Map_value<std::string, int>::parse("a=1,b=2,c=3");
  ASSERT_TRUE(result.has_value());
  
  const auto& values = result->values();
  EXPECT_EQ(values.size(), 3);
  EXPECT_EQ(values.at("a"), 1);
  EXPECT_EQ(values.at("c"), 3);
}

TEST_F(Map_value_test, ToString) {
  cli::Map_value<std::string, int> map({{"a", 1}, {"b", 2}});
  EXPECT_EQ(map.to_string(), "{a=1},{b=2}");

  cli::Map_value<std::string, std::string> str_map({{"key1", "value1"}, {"key2", "value2"}});
  EXPECT_EQ(str_map.to_string(), "{key1=value1},{key2=value2}");
}

TEST_F(Map_value_test, InvalidInput) {
  auto result = cli::Map_value<std::string, int>::parse("invalid=map=format");
  EXPECT_FALSE(result.has_value());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
