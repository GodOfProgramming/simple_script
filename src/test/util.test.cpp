#include "helpers.hpp"
#include "ss/util.hpp"
#include <gtest/gtest.h>

TEST(and_with, works)
{
  EXPECT_TRUE(ss::and_width(true));
  EXPECT_TRUE(ss::and_width(true, true, true, true));
  EXPECT_FALSE(ss::and_width(true, true, false, true));
}
