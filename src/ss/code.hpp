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
#define SS_ENUM_TO_STR_CASE(enum, name)                                                                                        \
  case enum ::name: {                                                                                                          \
    return #name;                                                                                                              \
  }

namespace ss
{
  enum class OpCode : std::uint8_t
  {
    /**
     * @brief No operation instruction
     */
    NO_OP,
    /**
     * @brief Looks up a constant value. The index is specified by the modifying bits
     */
    CONSTANT,
    /**
     * @brief Pushes a nil value on to the stack
     */
    NIL,
    /**
     * @brief Pushes a true value on to the stack
     */
    TRUE,
    /**
     * @brief Pushes a false value on to the stack
     */
    FALSE,
    /**
     * @brief Pops a value off the stack
     */
    POP,
    /**
     * @brief Pops N values off the stack. N is specified by the modifying bits
     */
    POP_N,
    /**
     * @brief Looks up a local variable. . The index in the stack is specified by the modifying bits
     */
    LOOKUP_LOCAL,
    /**
     * @brief Assigns a value to the local variable. The value comes off the top of the stack
     */
    ASSIGN_LOCAL,
    /**
     * @brief Looks up a global variable. The name is stored in the constant list. The index to then name is specified by the
     * modifying bits
     */
    LOOKUP_GLOBAL,
    /**
     * @brief Defines a new global variable. The name is stored in the constant list. The index to then name is specified by the
     * modifying bits
     */
    DEFINE_GLOBAL,
    /**
     * @brief Assigns a value to the global variable. The value comes off the top of the stack
     */
    ASSIGN_GLOBAL,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    EQUAL,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    NOT_EQUAL,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    GREATER,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    GREATER_EQUAL,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    LESS,
    /**
     * @brief Pops two values off the stack, compares, then pushes the result back on
     */
    LESS_EQUAL,
    /**
     * @brief Pops two values off the stack, calculates the sum, then pushes the result back on
     */
    ADD,
    /**
     * @brief Pops two values off the stack, calculates the difference, then pushes the result back on
     */
    SUB,
    /**
     * @brief Pops two values off the stack, calculates the product, then pushes the result back on
     */
    MUL,
    /**
     * @brief Pops two values off the stack, calculates the quotient, then pushes the result back on
     */
    DIV,
    /**
     * @brief Pops two values off the stack, calculates the remainder, then pushes the result back on
     */
    MOD,
    /**
     * @brief Pops a value off the stack, inverts its truthy value, then pushes that back on
     */
    NOT,
    /**
     * @brief Pops a value off the stack, inverts its numarical value, then pushes that back on
     */
    NEGATE,
    /**
     * @brief Pops a value off the stack and prints it to the screen
     */
    PRINT,
    /**
     * @brief TODO
     */
    RETURN,
  };

  struct Instruction
  {
    OpCode major_opcode = OpCode::NO_OP;
    // TODO set this up to be a union { struct { uint8_t, uint16_t, & uint32_t }; /* other combinations */ };
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
      SS_ENUM_TO_STR_CASE(OpCode, POP_N)
      SS_ENUM_TO_STR_CASE(OpCode, LOOKUP_LOCAL)
      SS_ENUM_TO_STR_CASE(OpCode, ASSIGN_LOCAL)
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

  class BytecodeChunk
  {
   public:
    using CodeSegment  = std::vector<Instruction>;
    using CodeIterator = CodeSegment::iterator;

    using IdentifierCache      = std::unordered_map<std::string_view, std::size_t>;
    using IdentifierCacheEntry = IdentifierCache::const_iterator;

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
     * @brief Pops values off the stack N times
     */
    void pop_stack_n(std::size_t n);

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
     * @brief Access values on the stack directly by index. Indexing behaves as normal
     *
     * @return The value accessed by the index. If the index is out of bounds, behavior is undefined
     */
    auto index_stack(std::size_t index) const noexcept -> Value;

    /**
     * @brief Access values on the stack directly by index. Indexing behaves as normal
     *
     * @return A mutable reference to the value accessed by the index. If the index is out of bounds, behavior is undefined
     */
    auto index_stack_mut(std::size_t index) noexcept -> Value&;

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

    auto find_ident(std::string_view name) const noexcept -> IdentifierCacheEntry;
    auto is_entry_found(IdentifierCacheEntry entry) const noexcept -> bool;

    /**
     * @brief Adds the identifier to the cache
     *
     * @return The index in the list of constants
     */
    auto add_ident(std::string_view name) noexcept -> std::size_t;

    auto begin() noexcept -> CodeIterator;
    auto end() noexcept -> CodeIterator;

   private:
    CodeSegment              code;
    std::vector<Value>       constants;
    std::vector<Value>       stack;
    std::vector<std::size_t> lines;
    std::size_t              last_line            = 0;
    std::size_t              instructions_on_line = 0;
    IdentifierCache          identifier_cache;

    void add_line(std::size_t line) noexcept;
  };

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

  struct Local
  {
    Token       name;
    std::size_t depth;
    bool        initialized;
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

    struct VarLookup
    {
      enum class Type
      {
        LOCAL,
        GLOBAL,
      };

      Type        type;
      std::size_t index;
    };

   public:
    Parser(TokenList&& tokens, BytecodeChunk& chunk) noexcept;
    ~Parser() = default;

    void parse();

   private:
    TokenList          tokens;
    TokenIterator      iter;
    BytecodeChunk&     chunk;
    std::vector<Local> locals;

    /**
     * @brief Current scope depth. 0 is the global namespace, depth > 0 creates local variables
     */
    std::size_t scope_depth;

    void write_instruction(Instruction i);

    auto previous() const -> TokenIterator;
    void advance() noexcept;
    void consume(Token::Type type, std::string err);
    void error(TokenIterator tok, std::string msg) const;
    void emit_instruction(Instruction i);
    void begin_scope();
    void end_scope();

    auto rule_for(Token::Type t) const noexcept -> const ParseRule&;
    void parse_precedence(Precedence p);
    void make_number(bool can_assign);
    void make_string(bool can_assign);
    void make_variable(bool assign);
    void named_variable(TokenIterator name, bool assign);
    auto parse_variable(std::string err_msg) -> std::size_t;
    /**
     * @brief Defines a new variable.
     *
     * @param global The index of the global variable in the chunk's constants. When defining a local variable, this will be 0
     */
    void define_variable(std::size_t global);
    void declare_variable();
    auto identifier_constant(TokenIterator name) -> std::size_t;
    auto check(Token::Type type) -> bool;
    auto advance_if_matches(Token::Type type) -> bool;
    void add_local(TokenIterator token) noexcept;
    auto resolve_local(TokenIterator token) const -> VarLookup;

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
    void block_statement();
  };

  class Compiler
  {
   public:
    Compiler() = default;
    void compile(std::string& src, BytecodeChunk& chunk);
  };

}  // namespace ss