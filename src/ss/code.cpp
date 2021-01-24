#include "code.hpp"

#include "datatypes.hpp"
#include "exceptions.hpp"
#include "util.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace ss
{
  auto operator<<(std::ostream& ostream, const OpCode& code) -> std::ostream&
  {
    return ostream << to_string(code);
  }

  auto Token::operator==(const Token& other) const noexcept -> bool
  {
    return this->type == other.type && this->lexeme == other.lexeme && this->line == other.line && this->column == other.column;
  }

  auto operator<<(std::ostream& ostream, const Token::Type& type) -> std::ostream&
  {
    return ostream << to_string(type);
  }

  auto operator<<(std::ostream& ostream, const Token& token) -> std::ostream&
  {
    return ostream << "{ type: " << token.type << ", lexeme: \"" << token.lexeme << "\", line: " << token.line
                   << ", column: " << token.column << " }";
  }

  void BytecodeChunk::prepare() noexcept
  {
    this->code.clear();
    this->constants.clear();
    this->stack.clear();
    this->lines.clear();
    this->last_line            = 0;
    this->instructions_on_line = 0;
    this->local_cache.clear();
    this->identifier_cache.clear();
  }

  void BytecodeChunk::write(Instruction i, std::size_t line) noexcept
  {
    this->code.push_back(i);
    this->add_line(line);
  }

  void BytecodeChunk::write_constant(Value v, std::size_t line) noexcept
  {
    this->constants.push_back(v);
    Instruction i{
     OpCode::CONSTANT,
     this->constants.size() - 1,
    };
    this->write(i, line);
  }

  auto BytecodeChunk::insert_constant(Value v) noexcept -> std::size_t
  {
    this->constants.push_back(v);
    return this->constants.size() - 1;
  }

  auto BytecodeChunk::constant_at(std::size_t offset) const noexcept -> Value
  {
    return this->constants[offset];
  }

  void BytecodeChunk::push_stack(Value v) noexcept
  {
    this->stack.push_back(v);
  }

  auto BytecodeChunk::pop_stack() noexcept -> Value
  {
    Value v = this->stack.back();
    this->stack.pop_back();
    return v;
  }

  void BytecodeChunk::pop_stack_n(std::size_t n)
  {
    this->stack.erase(this->stack.begin() + (this->stack.size() - n), this->stack.end());
  }

  auto BytecodeChunk::stack_empty() const noexcept -> bool
  {
    return this->stack.empty();
  }

  void BytecodeChunk::add_line(std::size_t line) noexcept
  {
    if (this->last_line == line) {
      // same line number
      this->instructions_on_line += 1;
    } else {
      this->lines.push_back(this->instructions_on_line);
      this->last_line            = line;
      this->instructions_on_line = 1;  // current instruction
    }
  }

  auto BytecodeChunk::line_at(std::size_t offset) const noexcept -> std::size_t
  {
    std::size_t accum = 0;
    std::size_t line  = 0;
    for (const auto num_instructions_on_line : this->lines) {
      if (accum + num_instructions_on_line > offset) {
        return line;
      } else {
        accum += num_instructions_on_line;
      }
      line++;
    }
    return line;
  }

  auto BytecodeChunk::peek_stack(std::size_t index) const noexcept -> Value
  {
    return this->stack[this->stack_size() - 1 - index];
  }

  auto BytecodeChunk::index_stack(std::size_t index) const noexcept -> Value
  {
    return this->stack[index];
  }

  auto BytecodeChunk::index_stack_mut(std::size_t index) noexcept -> Value&
  {
    return this->stack[index];
  }

  auto BytecodeChunk::stack_size() const noexcept -> std::size_t
  {
    return this->stack.size();
  }

  auto BytecodeChunk::instruction_count() const noexcept -> std::size_t
  {
    return this->code.size();
  }

  auto BytecodeChunk::index_code_mut(std::size_t index) -> Instruction&
  {
    return this->code[index];
  }

  auto BytecodeChunk::begin() noexcept -> CodeIterator
  {
    return this->code.begin();
  }

  auto BytecodeChunk::end() noexcept -> CodeIterator
  {
    return this->code.end();
  }

  auto BytecodeChunk::find_ident(std::string_view name) const noexcept -> IdentifierCacheEntry
  {
    return this->identifier_cache.find(name);
  }

  auto BytecodeChunk::is_entry_found(IdentifierCacheEntry entry) const noexcept -> bool
  {
    return entry != this->identifier_cache.end();
  }

  auto BytecodeChunk::add_ident(std::string_view name) noexcept -> std::size_t
  {
    auto indx                    = this->insert_constant(Value(std::string(name)));
    this->identifier_cache[name] = indx;
    return indx;
  }

  void BytecodeChunk::add_local(std::size_t index, std::string name)
  {
    this->local_cache.emplace(index, name);
  }

  auto BytecodeChunk::lookup_local(std::size_t index) -> std::string_view
  {
    return this->local_cache[index];
  }

  void BytecodeChunk::set_global(Value::StringType&& name, Value value) noexcept
  {
    this->globals[name] = value;
  }

  auto BytecodeChunk::find_global(Value::StringType name) noexcept -> GlobalMap::iterator
  {
    return this->globals.find(name);
  }

  auto BytecodeChunk::is_global_found(GlobalMap::iterator it) const noexcept -> bool
  {
    return it != this->globals.end();
  }

  void BytecodeChunk::print_stack(VMConfig& cfg) const noexcept
  {
    cfg.write("        | ");
    if (this->stack_empty()) {
      cfg.write_line("[ ]");
    } else {
      for (const auto& value : this->stack) { cfg.write("[ ", value.to_string(), " ]"); }
      cfg.write_line();
    }
  }

  void BytecodeChunk::print_constants(VMConfig& cfg) const noexcept
  {
    cfg.write_line("CONSTANTS");
    for (std::size_t i = 0; i < this->constants.size(); i++) { cfg.write_line(i, "=", this->constant_at(i)); }
  }

  void BytecodeChunk::print_local_map(VMConfig& cfg) const noexcept
  {
    cfg.write_line("LOCALS");
    for (const auto& pair : this->local_cache) { cfg.write_line(pair.first, "=", pair.second); }
  }

  Scanner::Scanner(std::string&& src) noexcept
   : source(std::move(src))
   , current_char(this->source.begin())
   , line(1)
   , column(1)
  {}

  auto Scanner::scan() -> std::vector<Token>
  {
    std::vector<Token> tokens;

    for (this->skip_whitespace(); !this->is_at_end(); this->skip_whitespace()) {
      char c = *this->starting_char;

      Token::Type t;

      switch (c) {
        case '(': {
          t = Token::Type::LEFT_PAREN;
        } break;
        case ')': {
          t = Token::Type::RIGHT_PAREN;
        } break;
        case '{': {
          t = Token::Type::LEFT_BRACE;
        } break;
        case '}': {
          t = Token::Type::RIGHT_BRACE;
        } break;
        case ',': {
          t = Token::Type::COMMA;
        } break;
        case '.': {
          t = Token::Type::DOT;
        } break;
        case ';': {
          t = Token::Type::SEMICOLON;
        } break;
        case '+': {
          t = Token::Type::PLUS;
        } break;
        case '-': {
          t = Token::Type::MINUS;
        } break;
        case '*': {
          t = Token::Type::STAR;
        } break;
        case '/': {
          t = Token::Type::SLASH;
        } break;
        case '%': {
          t = Token::Type::MODULUS;
        } break;
        case '!': {
          t = this->advance_if_match('=') ? Token::Type::BANG_EQUAL : Token::Type::BANG;
        } break;
        case '=': {
          t = this->advance_if_match('=') ? Token::Type::EQUAL_EQUAL
            : this->advance_if_match('>') ? Token::Type::ARROW
                                          : Token::Type::EQUAL;
        } break;
        case '<': {
          t = this->advance_if_match('=') ? Token::Type::LESS_EQUAL : Token::Type::LESS;
        } break;
        case '>': {
          t = this->advance_if_match('=') ? Token::Type::GREATER_EQUAL : Token::Type::GREATER;
        } break;
        case '"': {
          t = Token::Type::STRING;
        } break;
        default: {
          if (this->is_digit(c)) {
            t = Token::Type::NUMBER;
          } else if (this->is_alpha(c)) {
            t = Token::Type::IDENTIFIER;
          } else {
            std::stringstream ss;
            ss << "invalid character '" << *this->starting_char << '\'';
            this->error(ss.str());
          }
        }
      }

      this->advance();

      Token token;

      switch (t) {
        case Token::Type::STRING: {
          token = this->make_string();
        } break;
        case Token::Type::NUMBER: {
          token = this->make_number();
        } break;
        case Token::Type::IDENTIFIER: {
          token = this->make_identifier();
        } break;
        default: {
          token = this->make_token(t);
        } break;
      }

      tokens.push_back(token);
    }

    tokens.push_back(this->make_token(Token::Type::END_OF_FILE));

    return tokens;
  }

  void Scanner::error(std::string msg) const
  {
    std::stringstream ss;
    ss << this->line << ":" << this->column << " -> " << msg;
    THROW_COMPILETIME_ERROR(ss.str());
  }

  auto Scanner::make_token(Token::Type t) const noexcept -> Token
  {
    Token token;
    token.type   = t;
    token.lexeme = std::string_view(this->starting_char.base(), this->current_char - this->starting_char);
    token.line   = this->line;
    token.column = this->column - (this->current_char - this->starting_char);

    return token;
  }

  auto Scanner::make_string() -> Token
  {
    while (!this->is_at_end() && this->peek() != '"') {
      if (this->peek() == '\n') {
        this->line++;
      }
      this->advance();
    }

    if (this->is_at_end()) {
      this->error("unterminated string");
    }

    // advance past the leading '"'
    this->starting_char++;

    Token str = this->make_token(Token::Type::STRING);

    // advance past the closing '"'
    this->advance();

    return str;
  }

  auto Scanner::make_number() -> Token
  {
    while (this->is_digit(this->peek())) { this->advance(); }

    if (!this->is_at_end() && this->peek() == '.' && this->is_digit(this->peek_next())) {
      // advance past the "."
      this->advance();

      while (this->is_digit(this->peek())) { this->advance(); }
    }

    return this->make_token(Token::Type::NUMBER);
  }

  auto Scanner::make_identifier() -> Token
  {
    while (this->is_alpha(this->peek()) || this->is_digit(this->peek())) { this->advance(); }

    return this->make_token(this->identifier());
  }

  auto Scanner::identifier() -> Token::Type
  {
    switch (*this->starting_char) {
      case 'a': {
        return this->check_keyword(1, 2, "nd", Token::Type::AND);
      }
      case 'b': {
        return this->check_keyword(1, 4, "reak", Token::Type::BREAK);
      }
      case 'c': {
        switch (*(this->starting_char + 1)) {
          case 'l': {
            return this->check_keyword(2, 3, "ass", Token::Type::CLASS);
          }
          case 'o': {
            return this->check_keyword(2, 6, "ntinue", Token::Type::CONTINUE);
          }
          default:
            return Token::Type::IDENTIFIER;
        }
      }
      case 'e': {
        return this->check_keyword(1, 3, "lse", Token::Type::ELSE);
      }
      case 'f': {
        switch (*(this->starting_char + 1)) {
          case 'a': {
            return this->check_keyword(2, 3, "lse", Token::Type::FALSE);
          }
          case 'o': {
            return this->check_keyword(2, 1, "r", Token::Type::FOR);
          }
          case 'n': {
            return this->check_keyword(2, 0, "", Token::Type::FN);
          }
          default:
            return Token::Type::IDENTIFIER;
        }
      }
      case 'i': {
        return this->check_keyword(1, 1, "f", Token::Type::IF);
      }
      case 'l':
        switch (*(this->starting_char + 1)) {
          case 'e':
            return this->check_keyword(2, 1, "t", Token::Type::LET);
          case 'o':
            switch (*(this->starting_char + 2)) {
              case 'a':
                switch (*(this->starting_char + 3)) {
                  case 'd': {
                    auto type = this->check_keyword(4, 0, "", Token::Type::LOAD);
                    if (type == Token::Type::LOAD) {
                      return type;
                    } else {
                      switch (*(this->starting_char + 4)) {
                        case 'r':
                          return this->check_keyword(5, 0, "", Token::Type::LOADR);
                        default:
                          return Token::Type::IDENTIFIER;
                      }
                    }
                  }
                  default:
                    return Token::Type::IDENTIFIER;
                }
              case 'o':
                return this->check_keyword(3, 1, "p", Token::Type::LOOP);
              default:
                return Token::Type::IDENTIFIER;
            }
          default:
            return Token::Type::IDENTIFIER;
        }
      case 'm':
        return this->check_keyword(1, 4, "atch", Token::Type::MATCH);
      case 'n':
        return this->check_keyword(1, 2, "il", Token::Type::NIL);
      case 'o':
        return this->check_keyword(1, 1, "r", Token::Type::OR);
      case 'p':
        return this->check_keyword(1, 4, "rint", Token::Type::PRINT);
      case 'r':
        return this->check_keyword(1, 5, "eturn", Token::Type::RETURN);
      case 't':
        return this->check_keyword(1, 3, "rue", Token::Type::TRUE);
      case 'w':
        return this->check_keyword(1, 4, "hile", Token::Type::WHILE);
      default:
        return Token::Type::IDENTIFIER;
    }
  }

  auto Scanner::check_keyword(std::size_t start, std::size_t len, const char* rest, Token::Type type) const noexcept
   -> Token::Type
  {
    if (
     static_cast<std::size_t>(this->current_char - this->starting_char) == start + len &&
     std::memcmp((this->starting_char + start).base(), rest, len) == 0) {
      return type;
    }

    return Token::Type::IDENTIFIER;
  }

  auto Scanner::is_at_end() const noexcept -> bool
  {
    return this->current_char >= this->source.end();
  }

  auto Scanner::peek() const noexcept -> char
  {
    return *this->current_char;
  }

  auto Scanner::peek_next() const noexcept -> char
  {
    return *(this->current_char + 1);
  }

  auto Scanner::advance() noexcept -> char
  {
    char c = *this->current_char;
    this->current_char++;
    this->column++;
    return c;
  }

  auto Scanner::advance_if_match(char expected) noexcept -> bool
  {
    if (this->is_at_end() || this->peek_next() != expected) {
      return false;
    }

    this->advance();
    return true;
  }

  void Scanner::skip_whitespace() noexcept
  {
    bool done = false;
    while (!done && !this->is_at_end()) {
      switch (this->peek()) {
        case ' ':
        case '\r':
        case '\t': {
          this->advance();
        } break;
        case '\n': {
          this->line++;
          this->column = 1;
          this->advance();
        } break;
        case '#': {
          this->advance();
          while (!this->is_at_end() && this->advance() != '\n') {}
        } break;
        default: {
          done = true;
        }
      }
    }
    this->starting_char = this->current_char;
  }

  auto Scanner::is_digit(char c) const noexcept -> bool
  {
    return !this->is_at_end() && c >= '0' && c <= '9';
  }

  auto Scanner::is_alpha(char c) const noexcept -> bool
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@';
  }

  Parser::Parser(TokenList&& t, BytecodeChunk& c, std::string cf) noexcept
   : tokens(std::move(t))
   , iter(this->tokens.begin())
   , chunk(c)
   , current_file(cf)
   , scope_depth(0)
   , in_loop(false)
  {}

  void Parser::parse()
  {
    while (this->iter < tokens.end() && this->iter->type != Token::Type::END_OF_FILE) { this->declaration(); }
  }

  auto Parser::previous() const -> TokenIterator
  {
    return this->iter - 1;
  }

  void Parser::advance() noexcept
  {
    this->iter++;
  }

  void Parser::consume(Token::Type type, std::string err)
  {
    if (this->iter->type == type) {
      this->advance();
    } else {
      this->error(this->iter, err);
    }
  }

  void Parser::error(TokenIterator tok, std::string msg) const
  {
    std::stringstream ss;
    ss << tok->line << ":" << tok->column << " -> " << msg;
    THROW_COMPILETIME_ERROR(ss.str());
  }

  void Parser::emit_instruction(Instruction i)
  {
    this->chunk.write(i, this->previous()->line);
  }

  auto Parser::emit_jump(Instruction i) -> std::size_t
  {
    std::size_t location = this->chunk.instruction_count();
    this->emit_instruction(i);
    return location;
  }

  void Parser::patch_jump(std::size_t jump_loc)
  {
    std::size_t offset = this->chunk.instruction_count() - jump_loc;

    this->chunk.index_code_mut(jump_loc).modifying_bits = offset;
  }

  void Parser::wrap_scope(auto f)
  {
    this->scope_depth++;

    f();

    this->scope_depth--;

    std::size_t count = 0;
    while (!this->locals.empty() && this->locals.back().depth > this->scope_depth) {
      this->locals.pop_back();
      count++;
    }

    this->emit_instruction(Instruction{OpCode::POP_N, count});
  }

  void Parser::wrap_loop(std::size_t cont_jmp, auto f)
  {
    auto old_in_loop   = this->in_loop;
    auto old_depth     = this->loop_depth;
    auto old_breaks    = std::move(this->breaks);
    auto old_continue  = this->continue_jmp;
    this->in_loop      = true;
    this->continue_jmp = cont_jmp;
    this->loop_depth   = this->scope_depth;

    f();

    this->in_loop      = old_in_loop;
    this->loop_depth   = old_depth;
    this->breaks       = std::move(old_breaks);
    this->continue_jmp = old_continue;
  }

  auto Parser::rule_for(Token::Type t) const noexcept -> const ParseRule&
  {
    static const std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules = [this] {
      std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules{};
      rules[static_cast<std::size_t>(Token::Type::LEFT_PAREN)]  = {&Parser::grouping_expr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_PAREN)] = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LEFT_BRACE)]  = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_BRACE)] = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::COMMA)]       = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::DOT)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::SEMICOLON)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::PLUS)]        = {nullptr, &Parser::binary_expr, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::MINUS)]       = {&Parser::unary_expr, &Parser::binary_expr, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::STAR)]        = {nullptr, &Parser::binary_expr, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::SLASH)]       = {nullptr, &Parser::binary_expr, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::MODULUS)]     = {nullptr, &Parser::binary_expr, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::BANG)]        = {&Parser::unary_expr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::BANG_EQUAL)]  = {nullptr, &Parser::binary_expr, Precedence::EQUALITY};
      rules[static_cast<std::size_t>(Token::Type::EQUAL)]       = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::EQUAL_EQUAL)] = {nullptr, &Parser::binary_expr, Precedence::EQUALITY};
      rules[static_cast<std::size_t>(Token::Type::GREATER)]     = {nullptr, &Parser::binary_expr, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::GREATER_EQUAL)] = {nullptr, &Parser::binary_expr, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::LESS)]          = {nullptr, &Parser::binary_expr, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::LESS_EQUAL)]    = {nullptr, &Parser::binary_expr, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::ARROW)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::IDENTIFIER)]    = {&Parser::make_variable, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::STRING)]        = {&Parser::make_string, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NUMBER)]        = {&Parser::make_number, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::AND)]           = {nullptr, &Parser::and_expr, Precedence::AND};
      rules[static_cast<std::size_t>(Token::Type::BREAK)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::CLASS)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::CONTINUE)]      = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::ELSE)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FALSE)]         = {&Parser::literal_expr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FOR)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FN)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::IF)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LOAD)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LOADR)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LOOP)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::MATCH)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NIL)]           = {&Parser::literal_expr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::OR)]            = {nullptr, &Parser::or_expr, Precedence::OR};
      rules[static_cast<std::size_t>(Token::Type::PRINT)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RETURN)]        = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::TRUE)]          = {&Parser::literal_expr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LET)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::WHILE)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::ERROR)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::END_OF_FILE)]   = {nullptr, nullptr, Precedence::NONE};
      return rules;
    }();

    return rules[static_cast<std::size_t>(t)];
  }

  void Parser::parse_precedence(Precedence precedence)
  {
    this->advance();
    ParseFn prefix_rule = this->rule_for(this->previous()->type).prefix;
    if (prefix_rule == nullptr) {
      this->error(this->previous(), "expected an expression");
    }

    bool can_assign = precedence <= Precedence::ASSIGNMENT;
    prefix_rule(this, can_assign);

    while (precedence <= this->rule_for(this->iter->type).precedence) {
      this->advance();
      ParseFn infix_rule = this->rule_for(this->previous()->type).infix;
      infix_rule(this, can_assign);
    }

    if (can_assign && this->advance_if_matches(Token::Type::EQUAL)) {
      this->error(this->previous(), "invalid assignment target");
    }
  }

  void Parser::make_number(bool)
  {
    auto lexeme = this->previous()->lexeme;

    const char* begin = lexeme.data();

    // TODO when gcc finally has from_chars support for non-integrals
    char* end = const_cast<char*>(lexeme.data()) + lexeme.size();

    Value v = std::strtod(begin, &end);

    if (end == begin) {
      this->error(this->previous(), "unparsable number");
    }

    this->chunk.write_constant(v, this->previous()->line);
  }

  void Parser::make_string(bool)
  {
    Value v(std::string(this->previous()->lexeme));
    this->chunk.write_constant(v, this->previous()->line);
  }

  void Parser::make_variable(bool can_assign)
  {
    this->named_variable(this->previous(), can_assign);
  }

  void Parser::make_function(FnType type)
  {
    (void)type;
    // TODO figure out how to order function code better
    auto after = this->emit_jump(Instruction{OpCode::JUMP});
    this->wrap_scope([&] {
      this->consume(Token::Type::LEFT_PAREN, "expect '(' after function name");
      this->consume(Token::Type::RIGHT_PAREN, "expect ')' after parameters");
      this->consume(Token::Type::LEFT_BRACE, "expect '{' before function body");

      this->block_stmt();
    });
    this->patch_jump(after);
  }

  void Parser::named_variable(TokenIterator name, bool can_assign)
  {
    auto lookup = this->resolve_local(name);

    OpCode get, set;
    std::size_t index;
    if (lookup.type == VarLookup::Type::LOCAL) {
      get   = OpCode::LOOKUP_LOCAL;
      set   = OpCode::ASSIGN_LOCAL;
      index = lookup.index;
    } else if (lookup.type == VarLookup::Type::GLOBAL) {
      get   = OpCode::LOOKUP_GLOBAL;
      set   = OpCode::ASSIGN_GLOBAL;
      index = this->identifier_constant(name);
    } else {
      // impossible for now
      std::stringstream ss;
      ss << "invalid lookup type for var '" << name->lexeme << "'";
      THROW_RUNTIME_ERROR(ss.str());
    }

    if (can_assign && this->advance_if_matches(Token::Type::EQUAL)) {
      this->expression();
      this->emit_instruction(Instruction{set, index});
    } else {
      this->emit_instruction(Instruction{get, index});
    }
  }

  auto Parser::parse_variable(std::string err_msg) -> std::size_t
  {
    this->consume(Token::Type::IDENTIFIER, err_msg);
    this->declare_variable();
    return this->scope_depth > 0 ? 0 : this->identifier_constant(this->previous());
  }

  void Parser::define_variable(std::size_t global)
  {
    if (this->scope_depth == 0) {
      this->emit_instruction(Instruction{OpCode::DEFINE_GLOBAL, global});
    } else {
      this->locals.back().initialized = true;
    }
  }

  void Parser::declare_variable()
  {
    if (this->scope_depth > 0) {
      auto name = this->previous();
      for (auto local = this->locals.rbegin(); local != this->locals.rend(); local++) {
        if (local->initialized && local->depth < this->scope_depth) {
          break;
        }

        if (name->lexeme == local->name.lexeme) {
          this->error(name, "variable with same name already delcared in scope");
        }
      }
      this->add_local(name);
    }
  }

  auto Parser::identifier_constant(TokenIterator name) -> std::size_t
  {
    auto entry = this->chunk.find_ident(name->lexeme);
    if (this->chunk.is_entry_found(entry)) {
      return entry->second;
    } else {
      return this->chunk.add_ident(name->lexeme);
    }
  }

  auto Parser::check(Token::Type type) -> bool
  {
    return this->iter->type == type;
  }

  auto Parser::advance_if_matches(Token::Type type) -> bool
  {
    if (!this->check(type)) {
      return false;
    }

    this->advance();
    return true;
  }

  void Parser::add_local(TokenIterator name) noexcept
  {
    Local local;
    local.name        = *name;
    local.depth       = this->scope_depth;
    local.initialized = false;
    this->locals.push_back(local);

    if constexpr (DISASSEMBLE_CHUNK || DISASSEMBLE_INSTRUCTIONS) {
      this->chunk.add_local(this->locals.size() - 1, std::string(name->lexeme));
    }
  }

  auto Parser::resolve_local(TokenIterator name) const -> VarLookup
  {
    std::size_t index = this->locals.size() - 1;
    for (auto local = this->locals.rbegin(); local != this->locals.rend(); local++, index--) {
      if (name->lexeme == local->name.lexeme) {
        if (!local->initialized) {
          this->error(name, "can't read variable in it's own initializer");
        }
        return VarLookup{
         .type  = VarLookup::Type::LOCAL,
         .index = index,
        };
      }
    }

    return VarLookup{
     .type  = VarLookup::Type::GLOBAL,
     .index = 0,
    };
  }

  void Parser::expression()
  {
    this->parse_precedence(Precedence::ASSIGNMENT);
  }

  void Parser::grouping_expr(bool)
  {
    this->expression();
    this->consume(Token::Type::RIGHT_PAREN, "expect ')' after expression");
  }

  void Parser::unary_expr(bool)
  {
    Token::Type operator_type = this->previous()->type;

    this->parse_precedence(Precedence::UNARY);

    switch (operator_type) {
      case Token::Type::BANG: {
        this->emit_instruction(Instruction{OpCode::NOT});
      } break;
      case Token::Type::MINUS: {
        this->emit_instruction(Instruction{OpCode::NEGATE});
      } break;
      default:  // unreachable
        this->error(this->previous(), "invalid unary operator");
    }
  }

  void Parser::binary_expr(bool)
  {
    Token::Type operator_type = this->previous()->type;

    const ParseRule& rule = this->rule_for(operator_type);
    this->parse_precedence(static_cast<Precedence>(static_cast<std::size_t>(rule.precedence) + 1));

    switch (operator_type) {
      case Token::Type::EQUAL_EQUAL: {
        this->emit_instruction(Instruction{OpCode::EQUAL});
      } break;
      case Token::Type::BANG_EQUAL: {
        this->emit_instruction(Instruction{OpCode::NOT_EQUAL});
      } break;
      case Token::Type::GREATER: {
        this->emit_instruction(Instruction{OpCode::GREATER});
      } break;
      case Token::Type::GREATER_EQUAL: {
        this->emit_instruction(Instruction{OpCode::GREATER_EQUAL});
      } break;
      case Token::Type::LESS: {
        this->emit_instruction(Instruction{OpCode::LESS});
      } break;
      case Token::Type::LESS_EQUAL: {
        this->emit_instruction(Instruction{OpCode::LESS_EQUAL});
      } break;
      case Token::Type::PLUS: {
        this->emit_instruction(Instruction{OpCode::ADD});
      } break;
      case Token::Type::MINUS: {
        this->emit_instruction(Instruction{OpCode::SUB});
      } break;
      case Token::Type::STAR: {
        this->emit_instruction(Instruction{OpCode::MUL});
      } break;
      case Token::Type::SLASH: {
        this->emit_instruction(Instruction{OpCode::DIV});
      } break;
      case Token::Type::MODULUS: {
        this->emit_instruction(Instruction{OpCode::MOD});
      } break;
      default:  // unreachable
        this->error(this->previous(), "invalid binary operator");
    }
  }

  void Parser::literal_expr(bool)
  {
    switch (this->previous()->type) {
      case Token::Type::NIL: {
        this->emit_instruction(Instruction{OpCode::NIL});
      } break;
      case Token::Type::TRUE: {
        this->emit_instruction(Instruction{OpCode::TRUE});
      } break;
      case Token::Type::FALSE: {
        this->emit_instruction(Instruction{OpCode::FALSE});
      } break;
      default:  // unreachable
        THROW_COMPILETIME_ERROR("invalid literal type");
    }
  }

  void Parser::and_expr(bool)
  {
    std::size_t end_jmp = this->emit_jump(Instruction{OpCode::AND});
    this->parse_precedence(Precedence::AND);
    this->patch_jump(end_jmp);
  }

  void Parser::or_expr(bool)
  {
    std::size_t end_jmp = this->emit_jump(Instruction{OpCode::OR});
    this->parse_precedence(Precedence::OR);
    this->patch_jump(end_jmp);
  }

  void Parser::statement()
  {
    switch (this->iter->type) {
      case Token::Type::BREAK: {
        this->advance();
        this->break_stmt();
      } break;
      case Token::Type::CONTINUE: {
        this->advance();
        this->continue_stmt();
      } break;
      case Token::Type::FN: {
        this->advance();
        this->fn_stmt();
      } break;
      case Token::Type::FOR: {
        this->advance();
        this->for_stmt();
      } break;
      case Token::Type::IF: {
        this->advance();
        this->if_stmt();
      } break;
      case Token::Type::LEFT_BRACE: {
        this->advance();
        this->block_stmt();
      } break;
      case Token::Type::LOAD: {
        this->advance();
        this->load_stmt();
      } break;
      case Token::Type::LOADR: {
        this->advance();
        this->loadr_stmt();
      } break;
      case Token::Type::LOOP: {
        this->advance();
        this->loop_stmt();
      } break;
      case Token::Type::MATCH: {
        this->advance();
        this->match_stmt();
      } break;
      case Token::Type::PRINT: {
        this->advance();
        this->print_stmt();
      } break;
      case Token::Type::WHILE: {
        this->advance();
        this->while_stmt();
      } break;
      default: {
        this->expression_stmt();
      } break;
    }
  }

  void Parser::declaration()
  {
    if (this->advance_if_matches(Token::Type::LET)) {
      this->let_stmt();
    } else {
      this->statement();
    }
  }

  void Parser::print_stmt()
  {
    this->expression();
    this->consume(Token::Type::SEMICOLON, "expected ';' after value");
    this->emit_instruction(Instruction{OpCode::PRINT});
  }

  void Parser::expression_stmt()
  {
    this->expression();
    this->consume(Token::Type::SEMICOLON, "expected ';' after value");
    this->emit_instruction(Instruction{OpCode::POP});
  }

  void Parser::let_stmt()
  {
    std::size_t global = this->parse_variable("expect variable name");

    if (this->advance_if_matches(Token::Type::EQUAL)) {
      this->expression();
    } else {
      this->emit_instruction(Instruction{OpCode::NIL});
    }
    this->consume(Token::Type::SEMICOLON, "expect ';' after variable declaration");

    this->define_variable(global);
  }

  void Parser::block_stmt()
  {
    this->wrap_scope([&] {
      while (!this->check(Token::Type::RIGHT_BRACE) && !this->check(Token::Type::END_OF_FILE)) { this->declaration(); }
      this->consume(Token::Type::RIGHT_BRACE, "expect '}' after block");
    });
  }

  void Parser::if_stmt()
  {
    this->expression();
    this->consume(Token::Type::LEFT_BRACE, "expect '{' after condition");

    std::size_t jump_location = this->emit_jump(Instruction{OpCode::JUMP_IF_FALSE});
    this->emit_instruction(Instruction{OpCode::POP});
    this->block_stmt();

    std::size_t else_location = this->emit_jump(Instruction{OpCode::JUMP});
    this->patch_jump(jump_location);
    this->emit_instruction(Instruction{OpCode::POP});

    if (this->advance_if_matches(Token::Type::ELSE)) {
      this->statement();
    }

    this->patch_jump(else_location);
  }

  void Parser::loop_stmt()
  {
    std::size_t loop_start = this->chunk.instruction_count();
    this->consume(Token::Type::LEFT_BRACE, "expect '{' after loop keyword");
    this->wrap_loop(loop_start, [&] {
      this->block_stmt();
      this->emit_instruction(Instruction{OpCode::LOOP, this->chunk.instruction_count() - loop_start});
      for (const auto jmp : this->breaks) { this->patch_jump(jmp); }
    });
  }

  void Parser::while_stmt()
  {
    std::size_t loop_start = this->chunk.instruction_count();

    this->expression();
    this->consume(Token::Type::LEFT_BRACE, "expect '{' after condition");

    std::size_t exit_jmp = this->emit_jump(Instruction{OpCode::JUMP_IF_FALSE});

    this->emit_instruction(Instruction{OpCode::POP});
    this->wrap_loop(loop_start, [&] {
      this->block_stmt();

      this->emit_instruction(Instruction{OpCode::LOOP, this->chunk.instruction_count() - loop_start});

      this->patch_jump(exit_jmp);
      this->emit_instruction(Instruction{OpCode::POP});
      for (const auto jmp : this->breaks) { this->patch_jump(jmp); }
    });
  }

  void Parser::for_stmt()
  {
    this->wrap_scope([&] {
      if (this->advance_if_matches(Token::Type::SEMICOLON)) {
        // no initializer
      } else if (this->advance_if_matches(Token::Type::LET)) {
        this->let_stmt();
      } else {
        this->expression_stmt();
      }

      std::size_t loop_start = this->chunk.instruction_count();

      bool has_exit = false;
      std::size_t exit_jmp;

      if (!this->advance_if_matches(Token::Type::SEMICOLON)) {
        this->expression();
        this->consume(Token::Type::SEMICOLON, "expect ';'");

        has_exit = true;
        exit_jmp = this->emit_jump(Instruction{OpCode::JUMP_IF_FALSE});
        this->emit_instruction(Instruction{OpCode::POP});
      }

      // TODO consider pushing instructions to a separate vector and sticking them after the block stmt
      if (!this->advance_if_matches(Token::Type::LEFT_BRACE)) {
        std::size_t body_jmp = this->emit_jump(Instruction{OpCode::JUMP});

        std::size_t increment_start = this->chunk.instruction_count();
        this->expression();
        this->emit_instruction(Instruction{OpCode::POP});
        this->consume(Token::Type::LEFT_BRACE, "expect '}' after clauses");

        this->emit_instruction(Instruction{OpCode::LOOP, this->chunk.instruction_count() - loop_start});
        loop_start = increment_start;
        this->patch_jump(body_jmp);
      }

      this->wrap_loop(loop_start, [&] {
        this->block_stmt();

        this->emit_instruction(Instruction{OpCode::LOOP, this->chunk.instruction_count() - loop_start});

        if (has_exit) {
          this->patch_jump(exit_jmp);
          this->emit_instruction(Instruction{OpCode::POP});
          for (const auto jmp : this->breaks) { this->patch_jump(jmp); }
        }
      });
    });
  }

  void Parser::match_stmt()
  {
    this->expression();
    this->consume(Token::Type::LEFT_BRACE, "expect '{' after condition");

    while (!this->check(Token::Type::END_OF_FILE) && !this->check(Token::Type::RIGHT_BRACE)) {
      this->expression();
      this->consume(Token::Type::ARROW, "expect '=>' after expression");
      this->emit_instruction(Instruction{OpCode::CHECK});
      std::size_t next_jmp = this->emit_jump(Instruction{OpCode::JUMP_IF_FALSE});
      this->statement();
      this->patch_jump(next_jmp);
      this->emit_instruction(Instruction{OpCode::POP});
    }
    this->emit_instruction(Instruction{OpCode::POP});

    this->consume(Token::Type::RIGHT_BRACE, "expected '}' after match");
  }

  void Parser::break_stmt()
  {
    if (!this->in_loop) {
      this->error(this->previous(), "breaks can only be used within loops");
    }
    this->consume(Token::Type::SEMICOLON, "expect ';' after break");
    std::size_t count = 0;
    while (!this->locals.empty() && this->locals.back().depth > this->loop_depth) {
      this->locals.pop_back();
      count++;
    }
    this->emit_instruction(Instruction{OpCode::POP_N, count});
    this->breaks.push_back(this->emit_jump(Instruction{OpCode::JUMP}));
  }

  void Parser::continue_stmt()
  {
    if (!this->in_loop) {
      this->error(this->previous(), "continues can only be used within loops");
    }
    this->consume(Token::Type::SEMICOLON, "expect ';' after continue");
    std::size_t count = 0;
    while (!this->locals.empty() && this->locals.back().depth > this->loop_depth) {
      this->locals.pop_back();
      count++;
    }
    this->emit_instruction(Instruction{OpCode::POP_N, count});
    this->emit_instruction(Instruction{OpCode::LOOP, this->chunk.instruction_count() - this->continue_jmp});
  }

  void Parser::load_stmt()
  {
    if (this->scope_depth != 0) {
      this->error(this->iter, "can only load files in global scope");
    }
    this->consume(Token::Type::STRING, "expected file to be string type");
    auto file = this->previous()->lexeme;
    this->consume(Token::Type::SEMICOLON, "expected ';' after load stmt");
    auto libdirs    = std::getenv("SS_LIB");
    bool file_found = false;

    std::string dirs;
    if (libdirs == nullptr) {
      auto home = std::getenv("HOME");
      if (home != nullptr) {
        std::stringstream ss;
        ss << home << '/' << ".simple";
        dirs = ss.str();
      }
    } else {
      dirs = libdirs;
    }

    std::istringstream iss(dirs);
    std::string line;
    while (std::getline(iss, line, ':')) {
      std::stringstream ss;
      ss << line << '/' << file;
      std::string path = ss.str();
      if (std::filesystem::exists(path)) {
        auto contents = util::load_file_to_string(path);

        Compiler compiler;
        compiler.compile(std::move(contents), this->chunk, path);
        file_found = true;
      }
    }

    if (!file_found) {
      this->error(this->previous(), "unable to load file");
    }
  }

  void Parser::loadr_stmt()
  {
    if (this->scope_depth != 0) {
      this->error(this->iter, "can only load files in global scope");
    }
    this->consume(Token::Type::STRING, "expected file to be string type");
    auto file = this->previous()->lexeme;
    this->consume(Token::Type::SEMICOLON, "expected ';' after load stmt");
    std::filesystem::path path = this->current_file;
    std::stringstream ss;
    ss << path.parent_path().string() << '/' << file;
    path = ss.str();

    if (!std::filesystem::exists(path)) {
      this->error(this->previous(), "unable to load file");
    }

    auto contents = util::load_file_to_string(path.string());

    Compiler compiler;
    compiler.compile(std::move(contents), this->chunk, path.string());
  }

  void Parser::fn_stmt()
  {
    auto global = this->parse_variable("expected function name");
    if (this->scope_depth > 0) {
      this->locals.back().initialized = true;
    }
    this->make_function(FnType::FUNCTION);
    this->define_variable(global);
  }

  void Compiler::compile(std::string&& src, BytecodeChunk& chunk, std::string current_file)
  {
    Scanner scanner(std::move(src));

    auto tokens = scanner.scan();

    Parser parser(std::move(tokens), chunk, current_file);

    parser.parse();
  }
}  // namespace ss