#include "exceptions.hpp"
#include "lib.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
  constexpr bool DISASSEMBLE_CHUNK = false;
  constexpr bool SHOW_DISASSEMBLY  = false;
  constexpr bool PRINT_STACK       = false;
  constexpr bool ECHO_INPUT        = false;
}  // namespace

namespace ss
{
  VM::VM(VMConfig cfg): config(cfg), chunk(nullptr) {}

  auto VM::repl(VMConfig cfg) -> int
  {
    VM          vm(cfg);
    bool        exit        = false;
    int         exit_code   = 0;
    std::size_t line_number = 1;
    std::string line;

    while (!exit) {
      cfg.write("ss(main):", line_number, "> ");
      cfg.read_line(line);

      if (ECHO_INPUT) {
        cfg.write_line(line);
      }

      try {
        vm.run_script(line);
        line_number++;
      } catch (CompiletimeError& e) {
        cfg.write_line("compile error: ", e.what());
      } catch (RuntimeError& e) {
        cfg.write_line("runtime error: ", e.what());
      }
    }

    return exit_code;
  }

  void VM::run_script(std::string src)
  {
    Chunk chunk;

    compile(src, chunk);

    this->interpret(chunk);
  }

  void VM::run_chunk()
  {
    if constexpr (DISASSEMBLE_CHUNK) {
      this->disassemble_chunk("TODO", *this->chunk);
    }
    while (this->ip < this->chunk->code.end()) {
      if constexpr (SHOW_DISASSEMBLY) {
        if constexpr (PRINT_STACK) {
          this->print_stack();
        }
        this->disassemble_instruction(*this->chunk, *this->ip, this->ip - this->chunk->code.begin());
      }
      switch (static_cast<OpCode>(this->ip->major_opcode)) {
        case OpCode::NO_OP:
          break;
        case OpCode::CONSTANT: {
          this->chunk->push_stack(this->chunk->constant_at(this->ip->modifying_bits));
        } break;
        case OpCode::NIL: {
          this->chunk->push_stack(Value());
        } break;
        case OpCode::TRUE: {
          this->chunk->push_stack(Value(true));
        } break;
        case OpCode::FALSE: {
          this->chunk->push_stack(Value(false));
        } break;
        case OpCode::POP: {
          this->chunk->pop_stack();
        } break;
        case OpCode::EQUAL: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a == b);
        } break;
        case OpCode::NOT_EQUAL: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a != b);
        } break;
        case OpCode::GREATER: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a > b);
        } break;
        case OpCode::GREATER_EQUAL: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a >= b);
        } break;
        case OpCode::LESS: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a < b);
        } break;
        case OpCode::LESS_EQUAL: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a <= b);
        } break;
        case OpCode::ADD: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a + b);
        } break;
        case OpCode::SUB: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a - b);
        } break;
        case OpCode::MUL: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a * b);
        } break;
        case OpCode::DIV: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a / b);
        } break;
        case OpCode::MOD: {
          Value b = this->chunk->pop_stack();
          Value a = this->chunk->pop_stack();
          this->chunk->push_stack(a % b);
        } break;
        case OpCode::NOT: {
          this->chunk->push_stack(!this->chunk->pop_stack());
        } break;
        case OpCode::NEGATE: {
          this->chunk->push_stack(-this->chunk->pop_stack());
        } break;
        case OpCode::PRINT: {
          config.write_line(chunk->pop_stack());
        } break;
        case OpCode::RETURN: {
          return;
        } break;
        default: {
          std::stringstream ss;
          ss << "invalid op code: " << static_cast<std::uint32_t>(this->ip->major_opcode);
          THROW_RUNTIME_ERROR(ss.str());
        }
      }
      this->ip++;
    }
  }

  void VM::interpret(Chunk& chunk)
  {
    this->chunk = &chunk;
    this->ip    = this->chunk->code.begin();

    this->run_chunk();
  }

  void VM::disassemble_chunk(std::string name, Chunk& chunk) noexcept
  {
    this->config.write_line("== ", name, " ==");

    std::size_t offset = 0;
    for (const auto& i : chunk.code) {
      this->disassemble_instruction(chunk, i, offset++);
    }
  }

  void VM::disassemble_instruction(Chunk& chunk, Instruction i, std::size_t offset) noexcept
  {
#define SS_SIMPLE_PRINT_CASE(name)                    \
  case OpCode::name: {                                \
    this->config.write_line(to_string(OpCode::name)); \
  } break;

    this->config.write(std::setw(4), std::setfill('0'), offset, ' ');

    if (offset > 0 && chunk.line_at(offset) == chunk.line_at(offset - 1)) {
      this->config.write("   | ");
    } else {
      this->config.write(std::setw(4), std::setfill('0'), chunk.line_at(offset), ' ');
    }

    this->config.reset_ostream();

    switch (i.major_opcode) {
      SS_SIMPLE_PRINT_CASE(NO_OP)
      case OpCode::CONSTANT: {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, to_string(OpCode::CONSTANT));
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string().c_str(), "'\n");
        this->config.reset_ostream();
      } break;
        SS_SIMPLE_PRINT_CASE(NIL)
        SS_SIMPLE_PRINT_CASE(TRUE)
        SS_SIMPLE_PRINT_CASE(FALSE)
        SS_SIMPLE_PRINT_CASE(POP)
        SS_SIMPLE_PRINT_CASE(EQUAL)
        SS_SIMPLE_PRINT_CASE(NOT_EQUAL)
        SS_SIMPLE_PRINT_CASE(GREATER)
        SS_SIMPLE_PRINT_CASE(GREATER_EQUAL)
        SS_SIMPLE_PRINT_CASE(LESS)
        SS_SIMPLE_PRINT_CASE(LESS_EQUAL)
        SS_SIMPLE_PRINT_CASE(ADD)
        SS_SIMPLE_PRINT_CASE(SUB)
        SS_SIMPLE_PRINT_CASE(MUL)
        SS_SIMPLE_PRINT_CASE(DIV)
        SS_SIMPLE_PRINT_CASE(MOD)
        SS_SIMPLE_PRINT_CASE(NOT)
        SS_SIMPLE_PRINT_CASE(NEGATE)
        SS_SIMPLE_PRINT_CASE(PRINT)
        SS_SIMPLE_PRINT_CASE(RETURN)
      default: {
        this->config.write_line(to_string(i.major_opcode), ": ", static_cast<std::uint8_t>(i.major_opcode));
      } break;
    }

#undef SS_SIMPLE_PRINT_CASE
  }

  void VM::print_stack() noexcept
  {
    this->config.write("        | ");
    for (const auto& value : chunk->stack) {
      this->config.write("[ ", value.to_string(), " ]");
    }
    this->config.write_line();
  }
}  // namespace ss