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
  chunk.write_constant(Value(1.0), 1);
  chunk.write_constant(Value("str"), 2);

  EXPECT_EQ(chunk.line_at(0), 1);
  EXPECT_EQ(chunk.line_at(1), 1);
  EXPECT_EQ(chunk.line_at(2), 2);

  EXPECT_EQ(chunk.constant_at(0), Value());
  EXPECT_EQ(chunk.constant_at(1), Value(1.0));
  EXPECT_EQ(chunk.constant_at(2), Value("str"));
}

TEST(Chunk, METHOD(push_stack__pop_stack, can_push_onto_stack_and_pop))
{
  Chunk chunk;

  EXPECT_TRUE(chunk.stack_empty());

  chunk.push_stack(Value());
  chunk.push_stack(Value(1.0));
  chunk.push_stack(Value("str"));

  EXPECT_FALSE(chunk.stack_empty());

  EXPECT_EQ(chunk.pop_stack(), Value("str"));
  EXPECT_EQ(chunk.pop_stack(), Value(1.0));
  EXPECT_EQ(chunk.pop_stack(), Value());

  EXPECT_TRUE(chunk.stack_empty());
}

using ss::OpCode;

TEST(OpCode, METHOD(to_string, returns_the_right_string))
{
  EXPECT_STREQ(ss::to_string(OpCode::NO_OP), "NOOP");
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

  Scanner scanner(text);

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

  for (std::size_t i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i], tokens[i]) << "i: " << i;
  }
}

using ss::Parser;
using ss::Instruction;
using ss::OpCode;

TEST(Parser, METHOD(parse, some_math))
{
  std::string src = "!(5 - 4 > 3 * 2 == !nil)";
  Scanner scanner(src);

  auto tokens = scanner.scan();

  Chunk chunk;

  Parser parser(std::move(tokens), chunk);

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
  };

  ASSERT_EQ(expected.size(), chunk.code.size());
}
