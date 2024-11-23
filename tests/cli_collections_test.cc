#include <gtest/gtest.h>
#include <sstream>

#include "cli/cli.h"

class Array_options_test : public ::testing::Test {
protected:
  void SetUp() override {
    m_options = std::make_unique<cli::Options>();
  }

  void add_test_options() {
    // Add array options of different types
    m_options->add_option<cli::Array_value<int>>({
      .m_long_name = "numbers",
      .m_description = "List of numbers",
      .m_default_value = cli::Array_value<int>({1, 2, 3})
    });

    m_options->add_option<cli::Array_value<std::string>>({
      .m_long_name = "names",
      .m_description = "List of names",
      .m_default_value = cli::Array_value<std::string>({"alice", "bob", "charlie"})
    });

    m_options->add_option<cli::Array_value<bool>>({
      .m_long_name = "flags",
      .m_description = "List of boolean flags",
      .m_default_value = cli::Array_value<bool>({true, false, true, false})
    });

    m_options->add_option<cli::Array_value<double>>({
      .m_long_name = "values",
      .m_description = "List of decimal values",
      .m_default_value = cli::Array_value<double>({1.1, 2.2, 3.3})
    });
  }

  std::unique_ptr<cli::Options> m_options;
};

TEST_F(Array_options_test, DefaultValues) {
  add_test_options();
  const char* argv[] = {"program"};
  ASSERT_TRUE(m_options->parse(1, const_cast<char**>(argv)));

  auto numbers = m_options->get<cli::Array_value<int>>("numbers");
  ASSERT_TRUE(numbers.has_value());
  EXPECT_EQ(numbers->values().size(), 3);
  EXPECT_EQ(numbers->values()[0], 1);
  EXPECT_EQ(numbers->values()[2], 3);

  auto names = m_options->get<cli::Array_value<std::string>>("names");
  ASSERT_TRUE(names.has_value());
  EXPECT_EQ(names->values().size(), 3);
  EXPECT_EQ(names->values()[0], "alice");
  EXPECT_EQ(names->values()[2], "charlie");
}

TEST_F(Array_options_test, CommandLineOverride) {
  add_test_options();
  const char* argv[] = {
    "program",
    "--numbers=10,20,30",
    "--names=x,y,z"
  };
  ASSERT_TRUE(m_options->parse(3, const_cast<char**>(argv)));

  auto numbers = m_options->get<cli::Array_value<int>>("numbers");
  ASSERT_TRUE(numbers.has_value());
  EXPECT_EQ(numbers->values().size(), 3);
  EXPECT_EQ(numbers->values()[0], 10);
  EXPECT_EQ(numbers->values()[2], 30);

  auto names = m_options->get<cli::Array_value<std::string>>("names");
  ASSERT_TRUE(names.has_value());
  EXPECT_EQ(names->values().size(), 3);
  EXPECT_EQ(names->values()[0], "x");
  EXPECT_EQ(names->values()[2], "z");
}

TEST_F(Array_options_test, EmptyArrays) {
  m_options->add_option<cli::Array_value<int>>({
    .m_long_name = "empty",
    .m_description = "Empty array"
  });

  const char* argv[] = {"program", "--empty="};
  ASSERT_TRUE(m_options->parse(2, const_cast<char**>(argv)));

  auto empty = m_options->get<cli::Array_value<int>>("empty");
  EXPECT_TRUE(empty.has_value());
  EXPECT_TRUE(empty->values().empty());
}

TEST_F(Array_options_test, BooleanArrayParsing) {
  add_test_options();
  const char* argv[] = {
    "program",
    "--flags=true,1,yes,on,false,0,no,off"
  };
  ASSERT_TRUE(m_options->parse(2, const_cast<char**>(argv)));

  auto flags = m_options->get<cli::Array_value<bool>>("flags");
  ASSERT_TRUE(flags.has_value());
  EXPECT_EQ(flags->values().size(), 8);
  EXPECT_TRUE(flags->values()[0]);   // true
  EXPECT_TRUE(flags->values()[1]);   // 1
  EXPECT_TRUE(flags->values()[2]);   // yes
  EXPECT_TRUE(flags->values()[3]);   // on
  EXPECT_FALSE(flags->values()[4]);  // false
  EXPECT_FALSE(flags->values()[5]);  // 0
  EXPECT_FALSE(flags->values()[6]);  // no
  EXPECT_FALSE(flags->values()[7]);  // off
}

TEST_F(Array_options_test, InvalidValues) {
  m_options->add_option<cli::Array_value<int>>({
    .m_long_name = "numbers",
    .m_description = "List of numbers"
  });

  const char* argv[] = {"program", "--numbers=1,invalid,3"};
  EXPECT_FALSE(m_options->parse(2, const_cast<char**>(argv)));
}

class Map_options_test : public ::testing::Test {
protected:
  void SetUp() override {
    m_options = std::make_unique<cli::Options>();
  }

  void add_test_options() {
    m_options->add_option<cli::Map_value<std::string, int>>({
      .m_long_name = "limits",
      .m_description = "Resource limits",
      .m_default_value = cli::Map_value<std::string, int>({{"cpu", 4}, {"memory", 1024}, {"connections", 100}})
    });

    m_options->add_option<cli::Map_value<std::string, std::string>>({
      .m_long_name = "config",
      .m_description = "Configuration values",
      .m_default_value = cli::Map_value<std::string, std::string>({{"env", "prod"}, {"region", "us-west"}, {"tier", "premium"}})
    });

    m_options->add_option<cli::Map_value<std::string, bool>>({
      .m_long_name = "features",
      .m_description = "Feature flags",
      .m_default_value = cli::Map_value<std::string, bool>({{"logging", true}, {"debug", false}, {"verbose", true}})
    });

    m_options->add_option<cli::Map_value<std::string, double>>({
      .m_long_name = "metrics",
      .m_description = "Metric values",
      .m_default_value = cli::Map_value<std::string, double>({{"threshold", 0.85}, {"factor", 1.5}})
    });
  }

  std::unique_ptr<cli::Options> m_options;
};

TEST_F(Map_options_test, DefaultValues) {
  add_test_options();
  const char* argv[] = {"program"};
  ASSERT_TRUE(m_options->parse(1, const_cast<char**>(argv)));

  auto limits = m_options->get<cli::Map_value<std::string, int>>("limits");
  ASSERT_TRUE(limits.has_value());
  EXPECT_EQ(limits->values().size(), 3);
  EXPECT_EQ(limits->at("cpu"), 4);
  EXPECT_EQ(limits->at("memory"), 1024);
  EXPECT_EQ(limits->at("connections"), 100);

  auto config = m_options->get<cli::Map_value<std::string, std::string>>("config");
  ASSERT_TRUE(config.has_value());
  EXPECT_EQ(config->values().size(), 3);
  EXPECT_EQ(config->at("env"), "prod");
  EXPECT_EQ(config->at("region"), "us-west");
}

TEST_F(Map_options_test, CommandLineOverride) {
  add_test_options();
  const char* argv[] = {
    "program",
    "--limits=cpu=8,memory=2048",
    "--config=env=dev,region=eu-west"
  };
  ASSERT_TRUE(m_options->parse(3, const_cast<char**>(argv)));

  auto limits = m_options->get<cli::Map_value<std::string, int>>("limits");
  ASSERT_TRUE(limits.has_value());
  EXPECT_EQ(limits->values().size(), 2);
  EXPECT_EQ(limits->at("cpu"), 8);
  EXPECT_EQ(limits->at("memory"), 2048);

  auto config = m_options->get<cli::Map_value<std::string, std::string>>("config");
  ASSERT_TRUE(config.has_value());
  EXPECT_EQ(config->values().size(), 2);
  EXPECT_EQ(config->at("env"), "dev");
  EXPECT_EQ(config->at("region"), "eu-west");
}

TEST_F(Map_options_test, EmptyMap) {
  m_options->add_option<cli::Map_value<std::string, int>>({
    .m_long_name = "empty",
    .m_description = "Empty map"
  });

  const char* argv[] = {"program", "--empty="};
  ASSERT_TRUE(m_options->parse(2, const_cast<char**>(argv)));

  auto empty = m_options->get<cli::Map_value<std::string, int>>("empty");
  ASSERT_TRUE(empty.has_value());
  EXPECT_TRUE(empty->values().empty());
}

TEST_F(Map_options_test, BooleanMapParsing) {
  add_test_options();
  const char* argv[] = {
    "program",
    "--features=a=true,b=1,c=yes,d=on,e=false,f=0,g=no,h=off"
  };
  ASSERT_TRUE(m_options->parse(2, const_cast<char**>(argv)));

  auto features = m_options->get<cli::Map_value<std::string, bool>>("features");
  ASSERT_TRUE(features.has_value());
  EXPECT_EQ(features->values().size(), 8);
  EXPECT_TRUE(features->at("a"));   // true
  EXPECT_TRUE(features->at("b"));   // 1
  EXPECT_TRUE(features->at("c"));   // yes
  EXPECT_TRUE(features->at("d"));   // on
  EXPECT_FALSE(features->at("e"));  // false
  EXPECT_FALSE(features->at("f"));  // 0
  EXPECT_FALSE(features->at("g"));  // no
  EXPECT_FALSE(features->at("h"));  // off
}

TEST_F(Map_options_test, InvalidKeyValuePairs) {
  m_options->add_option<cli::Map_value<std::string, int>>({
    .m_long_name = "invalid",
    .m_description = "Invalid map"
  });

  const char* argv[] = {"program", "--invalid=key1:value1,key2=invalid"};
  EXPECT_FALSE(m_options->parse(2, const_cast<char**>(argv)));
}

TEST_F(Map_options_test, WhitespaceHandling) {
  m_options->add_option<cli::Map_value<std::string, int>>({
    .m_long_name = "spacing",
    .m_description = "Map with whitespace"
  });

  const char* argv[] = {"program", "--spacing= key1 = 100 , key2 = 200 "};
  ASSERT_TRUE(m_options->parse(2, const_cast<char**>(argv)));

  auto spacing = m_options->get<cli::Map_value<std::string, int>>("spacing");
  ASSERT_TRUE(spacing.has_value());
  EXPECT_EQ(spacing->values().size(), 2);
  EXPECT_EQ(spacing->at("key1"), 100);
  EXPECT_EQ(spacing->at("key2"), 200);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

