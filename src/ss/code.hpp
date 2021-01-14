#pragma once

#include "datatypes.hpp"
#include <cinttypes>
#include <string>
#include <vector>

namespace ss
{
  enum class OpCode : std::uint8_t
  {
    NO_OP,
    CONSTANT,
    ADD,
    SUB,
    MUL,
    DIV,
    NEGATE,
    RETURN,
  };

  struct Instruction
  {
    OpCode      major_opcode   = OpCode::NO_OP;
    std::size_t modifying_bits = 0;
  };

  constexpr auto to_string(OpCode op) noexcept -> const char*
  {
    switch (op) {
      case OpCode::NO_OP: {
        return "NOOP";
      }
      case OpCode::CONSTANT: {
        return "CONSTANT";
      }
      case OpCode::ADD: {
        return "ADD";
      } break;
      case OpCode::SUB: {
        return "SUB";
      } break;
      case OpCode::MUL: {
        return "MUL";
      } break;
      case OpCode::DIV: {
        return "DIV";
      } break;
      case OpCode::NEGATE: {
        return "NEGATE";
      }
      case OpCode::RETURN: {
        return "RETURN";
      }
      default: {
        return "UNKNOWN";
      }
    }
  }

  enum class TokenType
  {
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,

    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FN,
    IF,
    LET,
    NIL,
    OR,
    PRINT,
    RETURN,
    TRUE,
    WHILE,

    ERROR,
    END_OF_FILE,
  };

  struct Token
  {
    TokenType        type;
    std::string_view lexeme;
    std::size_t      line;
  };

  class Chunk
  {
   public:
    void write(Instruction, std::size_t line);
    void write_constant(Value v, std::size_t line);

    auto constant_at(std::size_t offset) -> Value;

    void push_stack(Value v);
    auto pop_stack() -> Value;
    auto stack_empty() -> bool;

    auto line_at(std::size_t offset) const noexcept -> std::size_t;

    std::vector<Instruction> code;
    std::vector<Value>       constants;
    std::vector<Value>       stack;

   private:
    std::vector<std::size_t> lines;
    std::size_t              last_line            = 0;
    std::size_t              instructions_on_line = 0;

    void add_line(std::size_t line) noexcept;
  };

  void compile(std::string& src, Chunk& chunk);

  class Scanner
  {
   public:
    Scanner(std::string& src) noexcept;
    ~Scanner() = default;

    auto scan() -> std::vector<Token>;

   private:
    std::string&          source;
    std::string::iterator start;
    std::string::iterator current;
    std::size_t           line;

    auto make_token(TokenType t) const noexcept -> Token;
    auto make_string() -> Token;
    auto make_number() -> Token;
    auto make_identifier() -> Token;
    auto identifier() -> TokenType;
    auto check_keyword(std::size_t start, std::size_t len, const char* rest, TokenType type) const noexcept -> TokenType;
    auto is_at_end() const noexcept -> bool;
    auto peek() const noexcept -> char;
    auto peek_next() const noexcept -> char;
    auto advance() noexcept -> char;
    auto advance_if_match(char expected) noexcept -> bool;
    void skip_whitespace() noexcept;
    auto is_digit(char c) const noexcept -> bool;
    auto is_alpha(char c) const noexcept -> bool;
  };

  class Parser
  {
    using TokenList     = std::vector<Token>;
    using TokenIterator = TokenList::iterator;

   public:
    Parser(TokenList&& tokens, Chunk& chunk) noexcept;
    ~Parser() = default;

    void parse();

   private:
    TokenList     tokens;
    TokenIterator iter;
    Chunk&        chunk;

    auto previous() const -> TokenIterator;
    void advance() noexcept;
    void consume(TokenType type, std::string err);
    void error(TokenIterator tok, std::string msg) const;
    void emit_instruction(Instruction i);

    void parse_number();
    void expression();
  };
}  // namespace ss