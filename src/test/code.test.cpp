#include "ss/code.hpp"

#include "helpers.hpp"

#include <gtest/gtest.h>

using ss::BytecodeChunk;
using ss::Instruction;
using ss::OpCode;
using ss::Value;

class TestBytecodeChunk: public testing::Test
{
 protected:
  BytecodeChunk chunk;
};

TEST_F(TestBytecodeChunk, METHOD(write, writing_adds_the_correct_line))
{
  this->chunk.write(Instruction{OpCode::RETURN}, 1);
  this->chunk.write(Instruction{OpCode::RETURN}, 1);
  this->chunk.write(Instruction{OpCode::RETURN}, 2);

  EXPECT_EQ(this->chunk.line_at(0), 1);
  EXPECT_EQ(this->chunk.line_at(1), 1);
  EXPECT_EQ(this->chunk.line_at(2), 2);
}

TEST_F(TestBytecodeChunk, METHOD(write_constant, can_write_constant))
{
  this->chunk.write_constant(Value(), 1);
  this->chunk.write_constant(Value(1.0), 1);
  this->chunk.write_constant(Value("str"), 2);

  EXPECT_EQ(this->chunk.line_at(0), 1);
  EXPECT_EQ(this->chunk.line_at(1), 1);
  EXPECT_EQ(this->chunk.line_at(2), 2);

  EXPECT_EQ(this->chunk.constant_at(0), Value());
  EXPECT_EQ(this->chunk.constant_at(1), Value(1.0));
  EXPECT_EQ(this->chunk.constant_at(2), Value("str"));
}

TEST_F(TestBytecodeChunk, METHOD(push_stack__pop_stack, can_push_onto_stack_and_pop))
{
  EXPECT_TRUE(this->chunk.stack_empty());

  this->chunk.push_stack(Value());
  this->chunk.push_stack(Value(1.0));
  this->chunk.push_stack(Value("str"));

  EXPECT_FALSE(this->chunk.stack_empty());

  EXPECT_EQ(this->chunk.pop_stack(), Value("str"));
  EXPECT_EQ(this->chunk.pop_stack(), Value(1.0));
  EXPECT_EQ(this->chunk.pop_stack(), Value());

  EXPECT_TRUE(this->chunk.stack_empty());
}

TEST_F(TestBytecodeChunk, METHOD(pop_stack_n, removes_the_specified_range))
{
  for (int i = 0; i < 10; i++) { this->chunk.push_stack(Value(1.0 * i)); }

  EXPECT_EQ(this->chunk.stack_size(), 10);

  this->chunk.pop_stack_n(5);

  EXPECT_EQ(this->chunk.stack_size(), 5);
  for (int i = 4; i < 0; i--) { EXPECT_EQ(this->chunk.pop_stack(), Value(1.0 * i)); }
}

using ss::OpCode;

TEST(OpCode, METHOD(to_string, returns_the_right_string))
{
  EXPECT_STREQ(ss::to_string(OpCode::NO_OP), "NO_OP");
  EXPECT_STREQ(ss::to_string(OpCode::CONSTANT), "CONSTANT");
  EXPECT_STREQ(ss::to_string(OpCode::NIL), "NIL");
  EXPECT_STREQ(ss::to_string(OpCode::TRUE), "TRUE");
  EXPECT_STREQ(ss::to_string(OpCode::FALSE), "FALSE");
  EXPECT_STREQ(ss::to_string(OpCode::ADD), "ADD");
  EXPECT_STREQ(ss::to_string(OpCode::SUB), "SUB");
  EXPECT_STREQ(ss::to_string(OpCode::MUL), "MUL");
  EXPECT_STREQ(ss::to_string(OpCode::DIV), "DIV");
  EXPECT_STREQ(ss::to_string(OpCode::MOD), "MOD");
  EXPECT_STREQ(ss::to_string(OpCode::NEGATE), "NEGATE");
  EXPECT_STREQ(ss::to_string(OpCode::RETURN), "RETURN");
  EXPECT_STREQ(ss::to_string(static_cast<OpCode>(-1)), "UNKNOWN");
}

using ss::Scanner;
using ss::Token;

TEST(Scanner, METHOD(scan, some_code))
{
  std::string text = "1 * 2 + 4 - 8 / 16";

  Scanner scanner(std::move(text));

  std::vector<Token> expected = {
   Token{
    Token::Type::NUMBER,
    std::string_view("1"),
    1,
    1,
   },
   Token{
    Token::Type::STAR,
    std::string_view("*"),
    1,
    3,
   },
   Token{
    Token::Type::NUMBER,
    std::string_view("2"),
    1,
    5,
   },
   Token{
    Token::Type::PLUS,
    std::string_view("+"),
    1,
    7,
   },
   Token{
    Token::Type::NUMBER,
    std::string_view("4"),
    1,
    9,
   },
   Token{
    Token::Type::MINUS,
    std::string_view("-"),
    1,
    11,
   },
   Token{
    Token::Type::NUMBER,
    std::string_view("8"),
    1,
    13,
   },
   Token{
    Token::Type::SLASH,
    std::string_view("/"),
    1,
    15,
   },
   Token{
    Token::Type::NUMBER,
    std::string_view("16"),
    1,
    17,
   },
   Token{
    Token::Type::END_OF_FILE,
    std::string_view(""),
    1,
    19,
   },
  };

  auto tokens = scanner.scan();

  ASSERT_EQ(expected.size(), tokens.size());

  for (std::size_t i = 0; i < expected.size(); i++) { EXPECT_EQ(expected[i], tokens[i]) << "i: " << i; }
}

using ss::Instruction;
using ss::Local;
using ss::OpCode;
using ss::Parser;

TEST(Parser, METHOD(parse, some_math))
{
  std::string src = "!(5 - 4 > 3 * 2 == !nil);";
  Scanner scanner(std::move(src));

  auto tokens = scanner.scan();

  BytecodeChunk chunk;

  Parser parser(std::move(tokens), chunk, "TEST");

  EXPECT_NO_THROW(parser.parse());

  std::vector<Instruction> expected = {
   Instruction{OpCode::CONSTANT, 0},
   Instruction{OpCode::CONSTANT, 1},
   Instruction{OpCode::SUB},
   Instruction{OpCode::CONSTANT, 2},
   Instruction{OpCode::CONSTANT, 3},
   Instruction{OpCode::MUL},
   Instruction{OpCode::GREATER},
   Instruction{OpCode::NIL},
   Instruction{OpCode::NOT},
   Instruction{OpCode::EQUAL},
   Instruction{OpCode::NOT},
   Instruction{OpCode::POP},
   Instruction{OpCode::NIL},
   Instruction{OpCode::END},
  };

  ASSERT_EQ(expected.size(), chunk.instruction_count());
}
