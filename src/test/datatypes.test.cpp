#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::Value;

TEST(Value, can_add_two_numbers)
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a + b, Value(3.0));
}

TEST(Value, can_add_number_and_string)
{
  Value a(1.2);
  Value b(" 2.3");

  EXPECT_EQ(a + b, Value("1.2 2.3"));
}

TEST(Value, can_add_string_and_number)
{
  Value a("1.2 ");
  Value b(2.3);

  EXPECT_EQ(a + b, Value("1.2 2.3"));
}

TEST(Value, can_add_strings)
{
  Value a("hello");
  Value b(" ");
  Value c("world");

  EXPECT_EQ(a + b + c, Value("hello world"));
}
