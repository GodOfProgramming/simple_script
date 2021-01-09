#include "helpers.hpp"
#include "ss/code.hpp"
#include <gtest/gtest.h>

using ss::OpCode;

TEST(OpCode, METHOD(to_string, returns_the_right_string))
{
  EXPECT_STREQ(to_string(OpCode::NO_OP), "NOOP");
  EXPECT_STREQ(to_string(OpCode::CONSTANT), "CONSTANT");
  EXPECT_STREQ(to_string(OpCode::ADD), "ADD");
  EXPECT_STREQ(to_string(OpCode::SUB), "SUB");
  EXPECT_STREQ(to_string(OpCode::MUL), "MUL");
  EXPECT_STREQ(to_string(OpCode::DIV), "DIV");
  EXPECT_STREQ(to_string(OpCode::NEGATE), "NEGATE");
  EXPECT_STREQ(to_string(OpCode::RETURN), "RETURN");
  EXPECT_STREQ(to_string(static_cast<OpCode>(-1)), "UNKNOWN");
}
