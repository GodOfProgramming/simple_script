#include "helpers.hpp"
#include "ss/cfg.hpp"
#include <gtest/gtest.h>
#include <sstream>

using ss::VMConfig;

TEST(VMConfig, METHOD(write, writes_to_output))
{
  std::ostringstream oss;
  VMConfig           cfg(&std::cin, &oss);

  cfg.write("hello world");
  EXPECT_EQ(oss.str(), "hello world");
  oss.str(std::string());
  cfg.write_line("hello world");
  EXPECT_EQ(oss.str(), "hello world\n");
}

TEST(VMConfig, METHOD(read, reads_from_input))
{
  std::istringstream iss;
  VMConfig           cfg(&iss);

  iss.str("123 example");

  int         n;
  std::string word;

  cfg.read(n);
  cfg.read(word);

  EXPECT_EQ(n, 123);
  EXPECT_EQ(word, "example");
}

TEST(VMConfig, METHOD(read_line, can_read_a_whole_line))
{
  GTEST_SKIP();

  std::istringstream iss;
  VMConfig           cfg(&iss);

  iss.str(std::string("a multiword sentence"));

  std::string line;

  cfg.read_line(line);

  EXPECT_EQ(line, "a multiword sentence");
}
