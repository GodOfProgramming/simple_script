#pragma once

#include "datatypes.hpp"
#include <cinttypes>
#include <string>
#include <vector>
#include <functional>

namespace ss
{
  enum class OpCode : std::uint8_t
  {
    NO_OP,
    CONSTANT,
    NIL,
    TRUE,
    FALSE,
    POP,
    EQUAL,
    NOT_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NOT,
    NEGATE,
    PRINT,
    RETURN,
  };

  struct Instruction
  {
    OpCode      major_opcode   = OpCode::NO_OP;
    std::size_t modifying_bits = 0;
  };

  constexpr auto to_string(OpCode op) noexcept -> const char*
  {
#define SS_TO_STR(name) \
  case OpCode::name: {  \
    return #name;       \
  }
    switch (op) {
      SS_TO_STR(NO_OP)
      SS_TO_STR(CONSTANT)
      SS_TO_STR(NIL)
      SS_TO_STR(TRUE)
      SS_TO_STR(FALSE)
      SS_TO_STR(POP)
      SS_TO_STR(EQUAL)
      SS_TO_STR(NOT_EQUAL)
      SS_TO_STR(GREATER)
      SS_TO_STR(GREATER_EQUAL)
      SS_TO_STR(LESS)
      SS_TO_STR(LESS_EQUAL)
      SS_TO_STR(ADD)
      SS_TO_STR(SUB)
      SS_TO_STR(MUL)
      SS_TO_STR(DIV)
      SS_TO_STR(MOD)
      SS_TO_STR(NOT)
      SS_TO_STR(NEGATE)
      SS_TO_STR(PRINT)
      SS_TO_STR(RETURN)
      default: {
        return "UNKNOWN";
      }
    }

#undef SS_TO_STR
  }

  struct Token
  {
    enum class Type
    {
      // Single-character tokens.
      LEFT_PAREN,
      RIGHT_PAREN,
      LEFT_BRACE,
      RIGHT_BRACE,
      COMMA,
      DOT,
      SEMICOLON,
      PLUS,
      MINUS,
      STAR,
      SLASH,
      MODULUS,

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
      LAST,
    };

    Type             type;
    std::string_view lexeme;
    std::size_t      line;
    std::size_t      column;

    auto operator==(const Token& other) const noexcept -> bool;
  };

  auto operator<<(std::ostream& ostream, const Token& token) -> std::ostream&;

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
    std::size_t           column;

    void error(std::string msg) const;
    auto make_token(Token::Type t) const noexcept -> Token;
    auto make_string() -> Token;
    auto make_number() -> Token;
    auto make_identifier() -> Token;
    auto identifier() -> Token::Type;
    auto check_keyword(std::size_t start, std::size_t len, const char* rest, Token::Type type) const noexcept -> Token::Type;
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

    enum class Precedence
    {
      NONE,
      ASSIGNMENT,  // =
      OR,          // or
      AND,         // and
      EQUALITY,    // == !=
      COMPARISON,  // < > <= >=
      TERM,        // + -
      FACTOR,      // / *
      UNARY,       // - !
      CALL,        // . ()
      PRIMARY,
    };

    using ParseFn = std::function<void(Parser*)>;

    struct ParseRule
    {
      ParseFn    prefix;
      ParseFn    infix;
      Precedence precedence;
    };

   public:
    Parser(TokenList&& tokens, Chunk& chunk) noexcept;
    ~Parser() = default;

    void parse();

   private:
    TokenList     tokens;
    TokenIterator iter;
    Chunk&        chunk;

    void write_instruction(Instruction i);

    auto previous() const -> TokenIterator;
    void advance() noexcept;
    void consume(Token::Type type, std::string err);
    void error(TokenIterator tok, std::string msg) const;
    void emit_instruction(Instruction i);

    void parse_precedence(Precedence p);
    auto rule_for(Token::Type t) const noexcept -> const ParseRule&;
    void parse_number();
    void parse_string();
    auto check(Token::Type type) -> bool;
    auto advance_if_matches(Token::Type type) -> bool;

    void expression();
    void grouping();
    void unary();
    void binary();
    void literal();

    void declaration();

    void statement();
    void print_statement();
    void expression_statement();
  };
}  // namespace ss