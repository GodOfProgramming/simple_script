#include "cfg.hpp"
#include "code.hpp"
#include "datatypes.hpp"
#include "exceptions.hpp"
#include <boost/convert.hpp>
#include <boost/convert/strtol.hpp>
#include <charconv>
#include <cstring>
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

  void Chunk::write(Instruction i, std::size_t line) noexcept
  {
    this->code.push_back(i);
    this->add_line(line);
  }

  void Chunk::write_constant(Value v, std::size_t line) noexcept
  {
    this->constants.push_back(v);
    Instruction i{
     OpCode::CONSTANT,
     this->constants.size() - 1,
    };
    this->write(i, line);
  }

  auto Chunk::insert_constant(Value v) noexcept -> std::size_t
  {
    this->constants.push_back(v);
    return this->constants.size() - 1;
  }

  auto Chunk::constant_at(std::size_t offset) const noexcept -> Value
  {
    return this->constants[offset];
  }

  void Chunk::push_stack(Value v) noexcept
  {
    this->stack.push_back(v);
  }

  auto Chunk::pop_stack() noexcept -> Value
  {
    Value v = this->stack.back();
    this->stack.pop_back();
    return v;
  }

  auto Chunk::stack_empty() const noexcept -> bool
  {
    return this->stack.empty();
  }

  void Chunk::add_line(std::size_t line) noexcept
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

  void Chunk::print_stack(VMConfig& cfg) const noexcept
  {
    cfg.write("        | ");
    for (const auto& value : this->stack) {
      cfg.write("[ ", value.to_string(), " ]");
    }
    cfg.write_line();
  }

  auto Chunk::line_at(std::size_t offset) const noexcept -> std::size_t
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

  void compile(std::string& src, Chunk& chunk)
  {
    Scanner scanner(src);

    auto tokens = scanner.scan();

    Parser parser(std::move(tokens), chunk);

    parser.parse();
  }

  Scanner::Scanner(std::string& src) noexcept: source(src), current(source.begin()), line(1), column(1) {}

  auto Scanner::scan() -> std::vector<Token>
  {
    std::vector<Token> tokens;

    for (this->skip_whitespace(); !this->is_at_end(); this->skip_whitespace()) {
      char c = *this->start;

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
          t = this->advance_if_match('=') ? Token::Type::EQUAL_EQUAL : Token::Type::EQUAL;
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
            ss << "invalid character '" << *this->start << '\'';
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
    token.lexeme = std::string_view(this->start.base(), this->current - this->start);
    token.line   = this->line;
    token.column = this->column - (this->current - this->start);

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
    this->start++;

    Token str = this->make_token(Token::Type::STRING);

    // advance past the closing '"'
    this->advance();

    return str;
  }

  auto Scanner::make_number() -> Token
  {
    while (this->is_digit(this->peek())) {
      this->advance();
    }

    if (!this->is_at_end() && this->peek() == '.' && this->is_digit(this->peek_next())) {
      // advance past the "."
      this->advance();

      while (this->is_digit(this->peek())) {
        this->advance();
      }
    }

    return this->make_token(Token::Type::NUMBER);
  }

  auto Scanner::make_identifier() -> Token
  {
    while (this->is_alpha(this->peek()) || this->is_digit(this->peek())) {
      this->advance();
    }

    return this->make_token(this->identifier());
  }

  auto Scanner::identifier() -> Token::Type
  {
    switch (*this->start) {
      case 'a': {
        return this->check_keyword(1, 2, "nd", Token::Type::AND);
      }
      case 'c': {
        return this->check_keyword(1, 4, "lass", Token::Type::CLASS);
      }
      case 'e': {
        return this->check_keyword(1, 3, "lse", Token::Type::ELSE);
      }
      case 'f': {
        switch (*(this->start + 1)) {
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
      case 'l': {
        return this->check_keyword(1, 2, "et", Token::Type::LET);
      }
      case 'n': {
        return this->check_keyword(1, 2, "il", Token::Type::NIL);
      }
      case 'o': {
        return this->check_keyword(1, 1, "r", Token::Type::OR);
      }
      case 'p': {
        return this->check_keyword(1, 4, "rint", Token::Type::PRINT);
      }
      case 'r': {
        return this->check_keyword(1, 5, "eturn", Token::Type::RETURN);
      }
      case 't': {
        return this->check_keyword(1, 3, "rue", Token::Type::TRUE);
      }
      case 'w': {
        return this->check_keyword(1, 4, "hile", Token::Type::WHILE);
      }
      default:
        return Token::Type::IDENTIFIER;
    }
  }

  auto Scanner::check_keyword(std::size_t start, std::size_t len, const char* rest, Token::Type type) const noexcept
   -> Token::Type
  {
    if (
     static_cast<std::size_t>(this->current - this->start) == start + len &&
     std::memcmp((this->start + start).base(), rest, len) == 0) {
      return type;
    }

    return Token::Type::IDENTIFIER;
  }

  auto Scanner::is_at_end() const noexcept -> bool
  {
    return this->current >= this->source.end();
  }

  auto Scanner::peek() const noexcept -> char
  {
    return *this->current;
  }

  auto Scanner::peek_next() const noexcept -> char
  {
    return *(this->current + 1);
  }

  auto Scanner::advance() noexcept -> char
  {
    char c = *this->current;
    this->current++;
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
          while (!this->is_at_end() && this->advance() != '\n') {
          }
        } break;
        default: {
          done = true;
        }
      }
    }
    this->start = this->current;
  }

  auto Scanner::is_digit(char c) const noexcept -> bool
  {
    return !this->is_at_end() && c >= '0' && c <= '9';
  }

  auto Scanner::is_alpha(char c) const noexcept -> bool
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@';
  }

  Parser::Parser(TokenList&& t, Chunk& c) noexcept: tokens(std::move(t)), chunk(c) {}

  void Parser::parse()
  {
    this->iter = this->tokens.begin();
    while (this->iter < tokens.end() && this->iter->type != Token::Type::END_OF_FILE) {
      this->declaration();
    }
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

  auto Parser::rule_for(Token::Type t) const noexcept -> const ParseRule&
  {
    static const std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules = [this] {
      std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules{};
      rules[static_cast<std::size_t>(Token::Type::LEFT_PAREN)]    = {&Parser::grouping, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_PAREN)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LEFT_BRACE)]    = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_BRACE)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::COMMA)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::DOT)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::SEMICOLON)]     = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::PLUS)]          = {nullptr, &Parser::binary, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::MINUS)]         = {&Parser::unary, &Parser::binary, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::STAR)]          = {nullptr, &Parser::binary, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::SLASH)]         = {nullptr, &Parser::binary, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::MODULUS)]       = {nullptr, &Parser::binary, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::BANG)]          = {&Parser::unary, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::BANG_EQUAL)]    = {nullptr, &Parser::binary, Precedence::EQUALITY};
      rules[static_cast<std::size_t>(Token::Type::EQUAL)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::EQUAL_EQUAL)]   = {nullptr, &Parser::binary, Precedence::EQUALITY};
      rules[static_cast<std::size_t>(Token::Type::GREATER)]       = {nullptr, &Parser::binary, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::GREATER_EQUAL)] = {nullptr, &Parser::binary, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::LESS)]          = {nullptr, &Parser::binary, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::LESS_EQUAL)]    = {nullptr, &Parser::binary, Precedence::COMPARISON};
      rules[static_cast<std::size_t>(Token::Type::IDENTIFIER)]    = {&Parser::make_variable, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::STRING)]        = {&Parser::make_string, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NUMBER)]        = {&Parser::make_number, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::AND)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::CLASS)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::ELSE)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FALSE)]         = {&Parser::literal, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FOR)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FN)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::IF)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NIL)]           = {&Parser::literal, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::OR)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::PRINT)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RETURN)]        = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::TRUE)]          = {&Parser::literal, nullptr, Precedence::NONE};
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

    prefix_rule(this);

    while (static_cast<size_t>(precedence) <= static_cast<std::size_t>(this->rule_for(this->iter->type).precedence)) {
      this->advance();
      ParseFn infix_rule = this->rule_for(this->previous()->type).infix;
      infix_rule(this);
    }
  }

  void Parser::make_number()
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

  void Parser::make_string()
  {
    Value v(std::string(this->previous()->lexeme));
    this->chunk.write_constant(v, this->previous()->line);
  }

  void Parser::make_variable()
  {
    this->named_variable(this->previous());
  }

  void Parser::named_variable(TokenIterator name)
  {
    std::size_t arg = this->identifier_constant(name);
    this->emit_instruction(Instruction{OpCode::LOOKUP_GLOBAL, arg});
  }

  auto Parser::parse_variable(std::string err_msg) -> std::size_t
  {
    this->consume(Token::Type::IDENTIFIER, err_msg);
    return this->identifier_constant(this->previous());
  }

  void Parser::define_variable(std::size_t global)
  {
    this->emit_instruction(Instruction{OpCode::DEFINE_GLOBAL, global});
  }

  auto Parser::identifier_constant(TokenIterator name) -> std::size_t
  {
    return this->chunk.insert_constant(Value(std::string(name->lexeme)));
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

  void Parser::expression()
  {
    this->parse_precedence(Precedence::ASSIGNMENT);
  }

  void Parser::grouping()
  {
    this->expression();
    this->consume(Token::Type::RIGHT_PAREN, "expect ')' after expression");
  }

  void Parser::unary()
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

  void Parser::binary()
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

  void Parser::literal()
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

  void Parser::statement()
  {
    if (this->advance_if_matches(Token::Type::PRINT)) {
      this->print_statement();
    } else {
      this->expression_statement();
    }
  }

  void Parser::declaration()
  {
    if (this->advance_if_matches(Token::Type::LET)) {
      this->let_statement();
    } else {
      this->statement();
    }
  }

  void Parser::print_statement()
  {
    this->expression();
    this->consume(Token::Type::SEMICOLON, "expected ';' after value");
    this->emit_instruction(Instruction{OpCode::PRINT});
  }

  void Parser::expression_statement()
  {
    this->expression();
    this->consume(Token::Type::SEMICOLON, "expected ';' after value");
    this->emit_instruction(Instruction{OpCode::POP});
  }

  void Parser::let_statement()
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
}  // namespace ss