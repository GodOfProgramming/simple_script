#include "helpers.hpp"
#include "ss/exceptions.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::RuntimeError;
using ss::Value;

TEST(Value, METHOD(number, when_a_number_returns_the_internal_value))
{
  Value v(1);
  EXPECT_EQ(v.number(), 1);
}

TEST(Value, METHOD(number, when_not_a_number_returns_0))
{
  Value v;
  EXPECT_EQ(v.number(), 0);
}

TEST(Value, METHOD(string, when_a_string_returns_the_internal_value))
{
  Value v("string");
  EXPECT_EQ(v.string(), "string");
}

TEST(Value, METHOD(string, when_not_a_string_returns_empty_string))
{
  Value v;
  EXPECT_EQ(v.string(), "");
}

TEST(Value, METHOD(to_string, when_nil_returns_the_word_nil))
{
  Value v;
  EXPECT_EQ(v.to_string(), "nil");
}

TEST(Value, METHOD(to_string, when_a_number_returns_string_repr))
{
  Value v(1.2345);
  EXPECT_EQ(v.to_string(), "1.2345");
}

TEST(Value, METHOD(to_string, when_string_returns_internal_value))
{
  Value v("string");
  EXPECT_EQ(v.to_string(), "string");
}

TEST(Value, METHOD(operator_negate, can_negate_numbers))
{
  Value v(1.0);
  EXPECT_EQ(-v, Value(-1.0));
}

TEST(Value, METHOD(operator_negate, can_not_negate_nil))
{
  Value v;
  EXPECT_THROW(-v, RuntimeError);
}

TEST(Value, METHOD(operator_negate, can_not_negate_string))
{
  Value v("string");
  EXPECT_THROW(-v, RuntimeError);
}

TEST(Value, METHOD(operator_add, can_add_two_numbers))
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a + b, Value(3.0));
}

TEST(Value, METHOD(operator_add, can_add_number_and_string))
{
  Value a(1.2);
  Value b(" 2.3");

  EXPECT_EQ(a + b, Value("1.2 2.3"));
}

TEST(Value, METHOD(operator_add, can_add_string_and_number))
{
  Value a("1.2 ");
  Value b(2.3);

  EXPECT_EQ(a + b, Value("1.2 2.3"));
}

TEST(Value, METHOD(operator_add, can_add_strings))
{
  Value a("hello");
  Value b(" ");
  Value c("world");

  EXPECT_EQ(a + b + c, Value("hello world"));
}

TEST(Value, METHOD(operator_add, can_not_add_invalid_types))
{
  Value nil;
  Value n(1.0);
  Value s("string");

  EXPECT_THROW(n + nil, RuntimeError);
  EXPECT_THROW(nil + n, RuntimeError);
  EXPECT_THROW(nil + s, RuntimeError);
  EXPECT_THROW(s + nil, RuntimeError);
}

TEST(Value, METHOD(operator_sub, can_sub_two_numbers))
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a - b, Value(-1.0));
}

TEST(Value, METHOD(operator_sub, can_not_sub_invalid_types))
{
  Value nil;
  Value n(1);
  Value s("string");

  EXPECT_THROW(n - nil, RuntimeError);
  EXPECT_THROW(n - s, RuntimeError);
  EXPECT_THROW(nil - n, RuntimeError);
  EXPECT_THROW(nil - s, RuntimeError);
  EXPECT_THROW(s - n, RuntimeError);
  EXPECT_THROW(s - nil, RuntimeError);
}

TEST(Value, METHOD(operator_mul, can_mul_two_numbers))
{
  Value a(2);
  Value b(3);

  EXPECT_EQ(a * b, Value(6));
}

TEST(Value, METHOD(operator_mul, can_mul_a_number_with_a_string))
{
  Value a(2);
  Value b("a");

  EXPECT_EQ(a * b, Value("aa"));
}

TEST(Value, METHOD(operator_mul, can_mul_a_string_with_a_number))
{
  Value a("a");
  Value b(3);

  EXPECT_EQ(a * b, Value("aaa"));
}

TEST(Value, METHOD(operator_mul, can_not_mul_invalid_types))
{
  Value nil;
  Value n(1);
  Value s("string");

  EXPECT_THROW(n * nil, RuntimeError);
  EXPECT_THROW(nil * n, RuntimeError);
  EXPECT_THROW(s * nil, RuntimeError);
  EXPECT_THROW(nil * s, RuntimeError);
}

TEST(Value, METHOD(operator_sub, can_div_two_numbers))
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a / b, Value(0.5));
}

TEST(Value, METHOD(operator_sub, can_not_div_invalid_types))
{
  Value nil;
  Value n(1);
  Value s("string");

  EXPECT_THROW(n / nil, RuntimeError);
  EXPECT_THROW(n / s, RuntimeError);
  EXPECT_THROW(nil / n, RuntimeError);
  EXPECT_THROW(nil / s, RuntimeError);
  EXPECT_THROW(s / n, RuntimeError);
  EXPECT_THROW(s / nil, RuntimeError);
}

TEST(Value, METHOD(assign_operator, can_assign))
{
  Value nil;
  Value num(1.0);
  Value str("s");

  Value x;

  x = "s";
  EXPECT_EQ(x, str);

  x = 1;
  EXPECT_EQ(x, num);

  x = Value::nil;
  EXPECT_EQ(x, nil);
}
