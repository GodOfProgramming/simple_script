#pragma once

#include "cfg.hpp"
#include "datatypes.hpp"
#include <cinttypes>
#include <string>
#include <vector>
#include <functional>

/**
 * @brief Case statement for any to_string() enum function
 *
 * @return The string representation for the given enum
 */
#define SS_ENUM_TO_STR_CASE(enum, name) \
  case enum ::name: {                   \
    return #name;                       \
  }

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
    LOOKUP_GLOBAL,
    DEFINE_GLOBAL,
    ASSIGN_GLOBAL,
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
    switch (op) {
      SS_ENUM_TO_STR_CASE(OpCode, NO_OP)
      SS_ENUM_TO_STR_CASE(OpCode, CONSTANT)
      SS_ENUM_TO_STR_CASE(OpCode, NIL)
      SS_ENUM_TO_STR_CASE(OpCode, TRUE)
      SS_ENUM_TO_STR_CASE(OpCode, FALSE)
      SS_ENUM_TO_STR_CASE(OpCode, POP)
      SS_ENUM_TO_STR_CASE(OpCode, LOOKUP_GLOBAL)
      SS_ENUM_TO_STR_CASE(OpCode, DEFINE_GLOBAL)
      SS_ENUM_TO_STR_CASE(OpCode, ASSIGN_GLOBAL)
      SS_ENUM_TO_STR_CASE(OpCode, EQUAL)
      SS_ENUM_TO_STR_CASE(OpCode, NOT_EQUAL)
      SS_ENUM_TO_STR_CASE(OpCode, GREATER)
      SS_ENUM_TO_STR_CASE(OpCode, GREATER_EQUAL)
      SS_ENUM_TO_STR_CASE(OpCode, LESS)
      SS_ENUM_TO_STR_CASE(OpCode, LESS_EQUAL)
      SS_ENUM_TO_STR_CASE(OpCode, ADD)
      SS_ENUM_TO_STR_CASE(OpCode, SUB)
      SS_ENUM_TO_STR_CASE(OpCode, MUL)
      SS_ENUM_TO_STR_CASE(OpCode, DIV)
      SS_ENUM_TO_STR_CASE(OpCode, MOD)
      SS_ENUM_TO_STR_CASE(OpCode, NOT)
      SS_ENUM_TO_STR_CASE(OpCode, NEGATE)
      SS_ENUM_TO_STR_CASE(OpCode, PRINT)
      SS_ENUM_TO_STR_CASE(OpCode, RETURN)
      default: {
        return "UNKNOWN";
      }
    }
  }

  auto operator<<(std::ostream& ostream, const OpCode& code) -> std::ostream&;

  /**
   * @brief Structure representing scanned tokens
   */
  struct Token
  {
    /**
     * @brief Enumeration representing types of tokens
     */
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

  constexpr auto to_string(Token::Type type) noexcept -> const char*
  {
    switch (type) {
      SS_ENUM_TO_STR_CASE(Token::Type, LEFT_PAREN)
      SS_ENUM_TO_STR_CASE(Token::Type, RIGHT_PAREN)
      SS_ENUM_TO_STR_CASE(Token::Type, LEFT_BRACE)
      SS_ENUM_TO_STR_CASE(Token::Type, RIGHT_BRACE)
      SS_ENUM_TO_STR_CASE(Token::Type, COMMA)
      SS_ENUM_TO_STR_CASE(Token::Type, DOT)
      SS_ENUM_TO_STR_CASE(Token::Type, SEMICOLON)
      SS_ENUM_TO_STR_CASE(Token::Type, PLUS)
      SS_ENUM_TO_STR_CASE(Token::Type, MINUS)
      SS_ENUM_TO_STR_CASE(Token::Type, STAR)
      SS_ENUM_TO_STR_CASE(Token::Type, SLASH)
      SS_ENUM_TO_STR_CASE(Token::Type, MODULUS)
      SS_ENUM_TO_STR_CASE(Token::Type, BANG)
      SS_ENUM_TO_STR_CASE(Token::Type, BANG_EQUAL)
      SS_ENUM_TO_STR_CASE(Token::Type, EQUAL)
      SS_ENUM_TO_STR_CASE(Token::Type, EQUAL_EQUAL)
      SS_ENUM_TO_STR_CASE(Token::Type, GREATER)
      SS_ENUM_TO_STR_CASE(Token::Type, GREATER_EQUAL)
      SS_ENUM_TO_STR_CASE(Token::Type, LESS)
      SS_ENUM_TO_STR_CASE(Token::Type, LESS_EQUAL)
      SS_ENUM_TO_STR_CASE(Token::Type, IDENTIFIER)
      SS_ENUM_TO_STR_CASE(Token::Type, STRING)
      SS_ENUM_TO_STR_CASE(Token::Type, NUMBER)
      SS_ENUM_TO_STR_CASE(Token::Type, AND)
      SS_ENUM_TO_STR_CASE(Token::Type, CLASS)
      SS_ENUM_TO_STR_CASE(Token::Type, ELSE)
      SS_ENUM_TO_STR_CASE(Token::Type, FALSE)
      SS_ENUM_TO_STR_CASE(Token::Type, FOR)
      SS_ENUM_TO_STR_CASE(Token::Type, FN)
      SS_ENUM_TO_STR_CASE(Token::Type, IF)
      SS_ENUM_TO_STR_CASE(Token::Type, LET)
      SS_ENUM_TO_STR_CASE(Token::Type, NIL)
      SS_ENUM_TO_STR_CASE(Token::Type, OR)
      SS_ENUM_TO_STR_CASE(Token::Type, PRINT)
      SS_ENUM_TO_STR_CASE(Token::Type, RETURN)
      SS_ENUM_TO_STR_CASE(Token::Type, TRUE)
      SS_ENUM_TO_STR_CASE(Token::Type, WHILE)
      SS_ENUM_TO_STR_CASE(Token::Type, ERROR)
      SS_ENUM_TO_STR_CASE(Token::Type, END_OF_FILE)
      SS_ENUM_TO_STR_CASE(Token::Type, LAST)
      default: {
        return "UNKNOWN";
      }
    }
  }

  auto operator<<(std::ostream& ostream, const Token::Type& type) -> std::ostream&;
  auto operator<<(std::ostream& ostream, const Token& token) -> std::ostream&;

  class State
  {
   public:
    using CodeSegment  = std::vector<Instruction>;
    using CodeIterator = CodeSegment::iterator;

    /**
     * @brief Writes the instruction and tags it with the line
     */
    void write(Instruction, std::size_t line) noexcept;

    /**
     * @brief Writes a constant instruction and tags the instruction with the line
     */
    void write_constant(Value v, std::size_t line) noexcept;

    /**
     * @brief Writes a constant to the constant buffer
     *
     * @return The offset of the newly inserted constant
     */
    auto insert_constant(Value v) noexcept -> std::size_t;

    /**
     * @brief Acquires the constant at the given index
     *
     * @return The value at the offset
     */
    auto constant_at(std::size_t offset) const noexcept -> Value;

    /**
     * @brief Pushes a new value onto the stack
     */
    void push_stack(Value v) noexcept;

    /**
     * @brief Pops a value off the stack
     *
     * @return The value popped off the stack
     */
    auto pop_stack() noexcept -> Value;

    /**
     * @brief Check if the stack is empty
     *
     * @return True if the stack is empty, false otherwise
     */
    auto stack_empty() const noexcept -> bool;

    /**
     * @brief Prints the stack to the given output stream
     */
    void print_stack(VMConfig& cfg) const noexcept;

    /**
     * @brief Access values on the stack by index. Index 0 being the hightest part
     *
     * @return The value accessed by the index. If the index is out of bounds, behavior is undefined
     */
    auto peek_stack(std::size_t index = 0) const noexcept -> Value;

    /**
     * @brief Get the number of items on the stack
     *
     * @return The number of items on the stack
     */
    auto stack_size() const noexcept -> std::size_t;

    /**
     * @brief Grabs the line at the given offset
     *
     * @return The line number
     */
    auto line_at(std::size_t offset) const noexcept -> std::size_t;

    auto instruction_count() const noexcept -> std::size_t;

    auto begin() noexcept -> CodeIterator;
    auto end() noexcept -> CodeIterator;

   private:
    CodeSegment              code;
    std::vector<Value>       constants;
    std::vector<Value>       stack;
    std::vector<std::size_t> lines;
    std::size_t              last_line            = 0;
    std::size_t              instructions_on_line = 0;

    void add_line(std::size_t line) noexcept;
  };

  void compile(std::string& src, State& state);

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

    using ParseFn = std::function<void(Parser*, bool)>;

    struct ParseRule
    {
      ParseFn    prefix;
      ParseFn    infix;
      Precedence precedence;
    };

   public:
    Parser(TokenList&& tokens, State& state) noexcept;
    ~Parser() = default;

    void parse();

   private:
    TokenList     tokens;
    TokenIterator iter;
    State&        state;

    void write_instruction(Instruction i);

    auto previous() const -> TokenIterator;
    void advance() noexcept;
    void consume(Token::Type type, std::string err);
    void error(TokenIterator tok, std::string msg) const;
    void emit_instruction(Instruction i);

    auto rule_for(Token::Type t) const noexcept -> const ParseRule&;
    void parse_precedence(Precedence p);
    void make_number(bool can_assign);
    void make_string(bool can_assign);
    void make_variable(bool assign);
    void named_variable(TokenIterator name, bool assign);
    auto parse_variable(std::string err_msg) -> std::size_t;
    void define_variable(std::size_t global);
    auto identifier_constant(TokenIterator name) -> std::size_t;
    auto check(Token::Type type) -> bool;
    auto advance_if_matches(Token::Type type) -> bool;

    void expression();
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);

    void declaration();

    void statement();
    void print_statement();
    void expression_statement();
    void let_statement();
  };
}  // namespace ss