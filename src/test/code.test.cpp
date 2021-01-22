#include "helpers.hpp"
#include "ss/code.hpp"
#include <gtest/gtest.h>

using ss::Instruction;
using ss::OpCode;
using ss::State;
using ss::Value;

class TestState: public testing::Test
{
 protected:
  State state;
};

TEST_F(TestState, METHOD(write, writing_adds_the_correct_line))
{
  this->state.write(Instruction{OpCode::RETURN}, 1);
  this->state.write(Instruction{OpCode::RETURN}, 1);
  this->state.write(Instruction{OpCode::RETURN}, 2);

  EXPECT_EQ(this->state.line_at(0), 1);
  EXPECT_EQ(this->state.line_at(1), 1);
  EXPECT_EQ(this->state.line_at(2), 2);
}

TEST_F(TestState, METHOD(write_constant, can_write_constant))
{
  this->state.write_constant(Value(), 1);
  this->state.write_constant(Value(1.0), 1);
  this->state.write_constant(Value("str"), 2);

  EXPECT_EQ(this->state.line_at(0), 1);
  EXPECT_EQ(this->state.line_at(1), 1);
  EXPECT_EQ(this->state.line_at(2), 2);

  EXPECT_EQ(this->state.constant_at(0), Value());
  EXPECT_EQ(this->state.constant_at(1), Value(1.0));
  EXPECT_EQ(this->state.constant_at(2), Value("str"));
}

TEST_F(TestState, METHOD(push_stack__pop_stack, can_push_onto_stack_and_pop))
{
  EXPECT_TRUE(this->state.stack_empty());

  this->state.push_stack(Value());
  this->state.push_stack(Value(1.0));
  this->state.push_stack(Value("str"));

  EXPECT_FALSE(this->state.stack_empty());

  EXPECT_EQ(this->state.pop_stack(), Value("str"));
  EXPECT_EQ(this->state.pop_stack(), Value(1.0));
  EXPECT_EQ(this->state.pop_stack(), Value());

  EXPECT_TRUE(this->state.stack_empty());
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

using ss::Instruction;
using ss::OpCode;
using ss::Parser;

TEST(Parser, METHOD(parse, some_math))
{
  std::string src = "!(5 - 4 > 3 * 2 == !nil);";
  Scanner     scanner(src);

  auto tokens = scanner.scan();

  State state;

  Parser parser(std::move(tokens), state);

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
  };

  ASSERT_EQ(expected.size(), state.instruction_count());
}
