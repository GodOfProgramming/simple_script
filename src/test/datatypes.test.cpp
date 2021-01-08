#include "ss/exceptions.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::RuntimeError;
using ss::Value;

TEST(Value, can_negate_numbers)
{
  Value a(1.0);
  EXPECT_EQ(-a, Value(-1.0));
}

TEST(Value, can_not_negate_nil)
{
  Value a;
  EXPECT_THROW(-a, RuntimeError);
}

TEST(Value, can_not_negate_string)
{
  Value a("string");
  EXPECT_THROW(-a, RuntimeError);
}

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

TEST(Value, can_not_add_nil_with_anything)
{
  Value nil;
  Value n(1.0);
  Value s("string");

  EXPECT_THROW(nil + n, RuntimeError);
  EXPECT_THROW(nil + s, RuntimeError);
  EXPECT_THROW(n + nil, RuntimeError);
  EXPECT_THROW(s + nil, RuntimeError);
}
