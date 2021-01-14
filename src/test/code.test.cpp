#include "helpers.hpp"
#include "ss/code.hpp"
#include <gtest/gtest.h>

using ss::Chunk;
using ss::Instruction;
using ss::OpCode;
using ss::Value;

TEST(Chunk, METHOD(write, writing_adds_the_correct_line))
{
  Chunk chunk;

  chunk.write(Instruction{OpCode::RETURN}, 1);
  chunk.write(Instruction{OpCode::RETURN}, 1);
  chunk.write(Instruction{OpCode::RETURN}, 2);

  EXPECT_EQ(chunk.line_at(0), 1);
  EXPECT_EQ(chunk.line_at(1), 1);
  EXPECT_EQ(chunk.line_at(2), 2);
}

TEST(Chunk, METHOD(write_constant, can_write_constant))
{
  Chunk chunk;

  chunk.write_constant(Value(), 1);
  chunk.write_constant(Value(1), 1);
  chunk.write_constant(Value("str"), 2);

  EXPECT_EQ(chunk.line_at(0), 1);
  EXPECT_EQ(chunk.line_at(1), 1);
  EXPECT_EQ(chunk.line_at(2), 2);

  EXPECT_EQ(chunk.constant_at(0), Value());
  EXPECT_EQ(chunk.constant_at(1), Value(1));
  EXPECT_EQ(chunk.constant_at(2), Value("str"));
}

TEST(Chunk, METHOD(push_stack__pop_stack, can_push_onto_stack_and_pop)) {
  Chunk chunk;

  EXPECT_TRUE(chunk.stack_empty());

  chunk.push_stack(Value());
  chunk.push_stack(Value(1));
  chunk.push_stack(Value("str"));

  EXPECT_FALSE(chunk.stack_empty());

  EXPECT_EQ(chunk.pop_stack(), Value("str"));
  EXPECT_EQ(chunk.pop_stack(), Value(1));
  EXPECT_EQ(chunk.pop_stack(), Value());

  EXPECT_TRUE(chunk.stack_empty());
}

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
