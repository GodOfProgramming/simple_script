#include "helpers.hpp"
#include "ss/exceptions.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::RuntimeError;
using ss::Value;

TEST(Value, METHOD(boolean, when_a_bool_returns_the_internal_value))
{
  Value v1(true);
  Value v2(false);
  EXPECT_TRUE(v1.as_bool());
  EXPECT_FALSE(v2.as_bool());
}

TEST(Value, METHOD(boolean, when_not_a_bool_returns_false_always))
{
  Value v;
  EXPECT_FALSE(v.as_bool());
}

TEST(Value, METHOD(as_int, when_an_int_returns_the_internal_value))
{
  Value v(Value::IntType{-1});
  EXPECT_EQ(v.as_int(), -1);
}

TEST(Value, METHOD(as_uint, when_an_uint_returns_the_internal_value))
{
  Value v(Value::UintType{1});
  EXPECT_EQ(v.as_uint(), 1);
}

TEST(Value, METHOD(number, when_a_number_returns_the_internal_value))
{
  Value v(1.0);
  EXPECT_EQ(v.as_number(), 1);
}

TEST(Value, METHOD(number, when_not_a_number_returns_0))
{
  Value v;
  EXPECT_EQ(v.as_number(), 0);
}

TEST(Value, METHOD(string, when_a_string_returns_the_internal_value))
{
  Value v("string");
  EXPECT_EQ(v.type(), Value::Type::String);
  EXPECT_EQ(v.as_string(), "string");
}

TEST(Value, METHOD(string, when_not_a_string_returns_empty_string))
{
  Value v;
  EXPECT_EQ(v.as_string(), "");
}

TEST(Value, METHOD(to_string, when_nil_returns_the_word_nil))
{
  Value v;
  EXPECT_EQ(v.to_string(), "nil");
}

TEST(Value, METHOD(to_string, when_bool_returns_string_repr))
{
  Value v1(true);
  Value v2(false);
  EXPECT_EQ(v1.to_string(), "true");
  EXPECT_EQ(v2.to_string(), "false");
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

TEST(Value, METHOD(truthy, when_nil_returns_false))
{
  Value v;
  EXPECT_FALSE(v.truthy());
}

TEST(Value, METHOD(truthy, when_bool_returns_internal_value))
{
  Value v1(true);
  Value v2(false);
  EXPECT_TRUE(v1.truthy());
  EXPECT_FALSE(v2.truthy());
}

TEST(Value, METHOD(truthy, everything_else_returns_true))
{
  Value v1(0.0);
  Value v2(1.0);
  Value v3(-1.0);
  Value v4("some string");

  EXPECT_TRUE(v1.truthy());
  EXPECT_TRUE(v2.truthy());
  EXPECT_TRUE(v3.truthy());
  EXPECT_TRUE(v4.truthy());
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

TEST(Value, METHOD(operator_not, can_inverse_truth))
{
  Value v1;
  Value v2(true);
  Value v3(false);
  Value v4(1.0);
  Value v5("some string");

  EXPECT_TRUE((!v1).as_bool());
  EXPECT_FALSE((!v2).as_bool());
  EXPECT_TRUE((!v3).as_bool());
  EXPECT_FALSE((!v4).as_bool());
  EXPECT_FALSE((!v5).as_bool());
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
  Value n(1.0);
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
  Value a(2.0);
  Value b(3.0);

  EXPECT_EQ(a * b, Value(6.0));
}

TEST(Value, METHOD(operator_mul, can_mul_a_number_with_a_string))
{
  Value a(2.0);
  Value b("a");

  EXPECT_EQ(a * b, Value("aa"));
}

TEST(Value, METHOD(operator_mul, can_mul_a_string_with_a_number))
{
  Value a("a");
  Value b(3.0);

  EXPECT_EQ(a * b, Value("aaa"));
}

TEST(Value, METHOD(operator_mul, can_not_mul_invalid_types))
{
  Value nil;
  Value n(1.0);
  Value s("string");

  EXPECT_THROW(n * nil, RuntimeError);
  EXPECT_THROW(nil * n, RuntimeError);
  EXPECT_THROW(s * nil, RuntimeError);
  EXPECT_THROW(nil * s, RuntimeError);
}

TEST(Value, METHOD(operator_div, can_div_two_numbers))
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a / b, Value(0.5));
}

TEST(Value, METHOD(operator_div, can_not_div_invalid_types))
{
  Value nil;
  Value n(1.0);
  Value s("string");

  EXPECT_THROW(n / nil, RuntimeError);
  EXPECT_THROW(n / s, RuntimeError);
  EXPECT_THROW(nil / n, RuntimeError);
  EXPECT_THROW(nil / s, RuntimeError);
  EXPECT_THROW(s / n, RuntimeError);
  EXPECT_THROW(s / nil, RuntimeError);
}

TEST(Value, METHOD(operator_mod, can_mod_two_numbers))
{
  Value a(1.0);
  Value b(2.0);
  EXPECT_EQ(a % b, Value(1.0));
}

TEST(Value, METHOD(operator_mod, can_not_mod_invalid_types))
{
  Value nil;
  Value b(true);
  Value n(1.0);
  Value s("string");

  EXPECT_THROW(b % n, RuntimeError);
  EXPECT_THROW(b % nil, RuntimeError);
  EXPECT_THROW(b % s, RuntimeError);
  EXPECT_THROW(n % b, RuntimeError);
  EXPECT_THROW(n % nil, RuntimeError);
  EXPECT_THROW(n % s, RuntimeError);
  EXPECT_THROW(nil % b, RuntimeError);
  EXPECT_THROW(nil % n, RuntimeError);
  EXPECT_THROW(nil % s, RuntimeError);
  EXPECT_THROW(s % b, RuntimeError);
  EXPECT_THROW(s % n, RuntimeError);
  EXPECT_THROW(s % nil, RuntimeError);
}

TEST(Value, METHOD(assign_operator, can_assign))
{
  Value nil;
  Value bool_true(true);
  Value bool_false(false);
  Value num(1.0);
  Value str("s");

  Value x;

  x = "s";
  EXPECT_EQ(x, str);

  x = 1.0;
  EXPECT_EQ(x, num);

  x = false;
  EXPECT_EQ(x, bool_false);

  x = true;
  EXPECT_EQ(x, bool_true);

  x = Value::nil;
  EXPECT_EQ(x, nil);
}
