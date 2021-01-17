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
  void Chunk::write(Instruction i, std::size_t line)
  {
    this->code.push_back(i);
    this->add_line(line);
  }

  void Chunk::write_constant(Value v, std::size_t line)
  {
    this->constants.push_back(v);
    Instruction i{
     OpCode::CONSTANT,
     this->constants.size() - 1,
    };
    this->write(i, line);
  }

  auto Chunk::constant_at(std::size_t offset) -> Value
  {
    return this->constants[offset];
  }

  void Chunk::push_stack(Value v)
  {
    this->stack.push_back(v);
  }

  auto Chunk::pop_stack() -> Value
  {
    Value v = this->stack.back();
    this->stack.pop_back();
    return v;
  }

  auto Chunk::stack_empty() -> bool
  {
    return this->stack.empty();
  }

  // increments the current number of instructions on a line
  // or publishes the number and resets the count
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

  // extracts the line at the given instruction offset
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

    for (this->skip_whitespace(); !this->is_at_end(); this->advance_then_skip_whitespace()) {
      char c = *this->start;

      switch (c) {
        case '(': {
          tokens.push_back(this->make_token(Token::Type::LEFT_PAREN));
        } break;
        case ')': {
          tokens.push_back(this->make_token(Token::Type::RIGHT_PAREN));
        } break;
        case '{': {
          tokens.push_back(this->make_token(Token::Type::LEFT_BRACE));
        } break;
        case '}': {
          tokens.push_back(this->make_token(Token::Type::RIGHT_BRACE));
        } break;
        case ';': {
          tokens.push_back(this->make_token(Token::Type::SEMICOLON));
        } break;
        case ',': {
          tokens.push_back(this->make_token(Token::Type::COMMA));
        } break;
        case '.': {
          tokens.push_back(this->make_token(Token::Type::DOT));
        } break;
        case '-': {
          tokens.push_back(this->make_token(Token::Type::MINUS));
        } break;
        case '+': {
          tokens.push_back(this->make_token(Token::Type::PLUS));
        } break;
        case '/': {
          tokens.push_back(this->make_token(Token::Type::SLASH));
        } break;
        case '*': {
          tokens.push_back(this->make_token(Token::Type::STAR));
        } break;
        case '!': {
          tokens.push_back(this->make_token(this->advance_if_match('=') ? Token::Type::BANG_EQUAL : Token::Type::BANG));
        } break;
        case '=': {
          tokens.push_back(this->make_token(this->advance_if_match('=') ? Token::Type::EQUAL_EQUAL : Token::Type::EQUAL));
        } break;
        case '<': {
          tokens.push_back(this->make_token(this->advance_if_match('=') ? Token::Type::LESS_EQUAL : Token::Type::LESS));
        } break;
        case '>': {
          tokens.push_back(this->make_token(this->advance_if_match('=') ? Token::Type::GREATER_EQUAL : Token::Type::GREATER));
        } break;
        case '"': {
          tokens.push_back(this->make_string());
        } break;
        default: {
          if (this->is_digit(c)) {
            tokens.push_back(this->make_number());
          } else if (this->is_alpha(c)) {
            tokens.push_back(this->make_identifier());
          } else {
            std::stringstream ss;
            ss << "invalid character '" << *this->start << '\'';
            this->error(ss.str());
          }
        }
      }
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
    token.column = this->column;

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
    return c;
  }

  auto Scanner::advance_if_match(char expected) noexcept -> bool
  {
    if (*this->current != expected || this->is_at_end()) {
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

  void Scanner::advance_then_skip_whitespace() noexcept
  {
    this->advance();
    this->skip_whitespace();
  }

  auto Scanner::is_digit(char c) const noexcept -> bool
  {
    return !this->is_at_end() && c >= '0' && c <= '9';
  }

  auto Scanner::is_alpha(char c) const noexcept -> bool
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  Parser::Parser(TokenList&& t, Chunk& c) noexcept: tokens(std::move(t)), chunk(c) {}

  void Parser::parse()
  {
    for (this->iter = tokens.begin(); this->iter < tokens.end(); this->advance()) {
      this->expression();
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

  void Parser::parse_number()
  {
    auto& lexeme = this->iter->lexeme;

    const char* begin = lexeme.data();

    // TODO when gcc finally has from_chars support for non-integrals
    char* end = const_cast<char*>(lexeme.data()) + lexeme.size();

    Value v = std::strtod(begin, &end);

    this->chunk.write_constant(v, this->iter->line);
  }

  void Parser::parse_precedence(Precedence precedence)
  {
    this->advance();
    ParseFn prefix_rule = this->get_rule(this->previous()->type).prefix;
    if (prefix_rule == nullptr) {
      this->error(this->previous(), "expected an expression");
    }

    prefix_rule(this);

    while (static_cast<size_t>(precedence) <= static_cast<std::size_t>(this->get_rule(this->iter->type).precedence)) {
      this->advance();
      ParseFn infix_rule = this->get_rule(this->previous()->type).infix;
      infix_rule(this);
    }
  }

  auto Parser::get_rule(Token::Type t) const noexcept -> const ParseRule&
  {
    static const std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules = [this] {
      std::array<ParseRule, static_cast<std::size_t>(Token::Type::LAST)> rules;
      rules[static_cast<std::size_t>(Token::Type::LEFT_PAREN)]    = {&Parser::grouping, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_PAREN)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LEFT_BRACE)]    = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RIGHT_BRACE)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::COMMA)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::DOT)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::MINUS)]         = {&Parser::unary, &Parser::binary, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::PLUS)]          = {nullptr, &Parser::binary, Precedence::TERM};
      rules[static_cast<std::size_t>(Token::Type::SEMICOLON)]     = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::SLASH)]         = {nullptr, &Parser::binary, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::STAR)]          = {nullptr, &Parser::binary, Precedence::FACTOR};
      rules[static_cast<std::size_t>(Token::Type::BANG)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::BANG_EQUAL)]    = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::EQUAL)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::EQUAL_EQUAL)]   = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::GREATER)]       = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::GREATER_EQUAL)] = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LESS)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LESS_EQUAL)]    = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::IDENTIFIER)]    = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::STRING)]        = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NUMBER)]        = {&Parser::parse_number, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::AND)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::CLASS)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::ELSE)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FALSE)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FOR)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::FN)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::IF)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::NIL)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::OR)]            = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::PRINT)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::RETURN)]        = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::TRUE)]          = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::LET)]           = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::WHILE)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::ERROR)]         = {nullptr, nullptr, Precedence::NONE};
      rules[static_cast<std::size_t>(Token::Type::END_OF_FILE)]   = {nullptr, nullptr, Precedence::NONE};

      return rules;
    }();

    return rules[static_cast<std::size_t>(t)];
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
      case Token::Type::MINUS: {
        this->chunk.write(Instruction{OpCode::NEGATE}, this->iter->line);
      } break;
      default:  // unreachable
        this->error(this->previous(), "invalid unary operator");
    }
  }

  void Parser::binary()
  {
    Token::Type operator_type = this->previous()->type;

    const ParseRule& rule = this->get_rule(operator_type);
    this->parse_precedence(static_cast<Precedence>(static_cast<std::size_t>(rule.precedence) + 1));

    switch (operator_type) {
      case Token::Type::PLUS: {
        this->chunk.write(Instruction{OpCode::ADD}, this->previous()->line);
      } break;
      case Token::Type::MINUS: {
        this->chunk.write(Instruction{OpCode::SUB}, this->previous()->line);
      } break;
      case Token::Type::STAR: {
        this->chunk.write(Instruction{OpCode::MUL}, this->previous()->line);
      } break;
      case Token::Type::SLASH: {
        this->chunk.write(Instruction{OpCode::DIV}, this->previous()->line);
      } break;
      default:  // unreachable
        this->error(this->previous(), "invalid binary operator");
    }
  }
}  // namespace ss