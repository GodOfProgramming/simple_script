#include "ss/util.hpp"

#include "helpers.hpp"

#include <gtest/gtest.h>

class TestUtil: public testing::Test
{};

TEST_F(TestUtil, stream_copies_to_string)
{
  std::string data = "some data";
  std::stringstream ss;
  ss << data;
  auto out = ss::util::stream_to_string(ss);
  EXPECT_EQ(data, out);
}
