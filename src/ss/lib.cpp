#include "exceptions.hpp"
#include "lib.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
  constexpr bool SHOW_DISASSEMBLY = true;
  constexpr bool PRINT_STACK      = true;
  constexpr bool ECHO_INPUT       = true;
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
        case OpCode::ADD: {
          Value a = this->chunk->pop_stack();
          Value b = this->chunk->pop_stack();
          this->chunk->push_stack(a + b);
        } break;
        case OpCode::SUB: {
          Value a = this->chunk->pop_stack();
          Value b = this->chunk->pop_stack();
          this->chunk->push_stack(a - b);
        } break;
        case OpCode::MUL: {
          Value a = this->chunk->pop_stack();
          Value b = this->chunk->pop_stack();
          this->chunk->push_stack(a * b);
        } break;
        case OpCode::DIV: {
          Value a = this->chunk->pop_stack();
          Value b = this->chunk->pop_stack();
          this->chunk->push_stack(a / b);
        } break;
        case OpCode::NEGATE: {
          this->chunk->push_stack(-this->chunk->pop_stack());
        } break;
        case OpCode::RETURN: {
          if (!this->chunk->stack_empty()) {
            this->config.write_line(this->chunk->pop_stack().to_string());
          }
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
    this->config.write(std::setw(4), std::setfill('0'), offset, ' ');

    if (offset > 0 && chunk.line_at(offset) == chunk.line_at(offset - 1)) {
      this->config.write("   | ");
    } else {
      this->config.write(std::setw(4), std::setfill('0'), chunk.line_at(offset), ' ');
    }

    this->config.reset_ostream();

    switch (i.major_opcode) {
      case OpCode::NO_OP: {
        this->config.write_line(to_string(OpCode::NO_OP));
      } break;
      case OpCode::CONSTANT: {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, to_string(OpCode::CONSTANT));
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string().c_str(), "'\n");
        this->config.reset_ostream();
      } break;
      case OpCode::ADD: {
        this->config.write_line(to_string(OpCode::ADD));
      } break;
      case OpCode::SUB: {
        this->config.write_line(to_string(OpCode::SUB));
      } break;
      case OpCode::MUL: {
        this->config.write_line(to_string(OpCode::MUL));
      } break;
      case OpCode::DIV: {
        this->config.write_line(to_string(OpCode::DIV));
      } break;
      case OpCode::NEGATE: {
        this->config.write_line(to_string(OpCode::NEGATE));
      } break;
      case OpCode::RETURN: {
        this->config.write_line(to_string(OpCode::RETURN));
      } break;
      default: {
        this->config.write_line(to_string(i.major_opcode), ": ", static_cast<std::uint8_t>(i.major_opcode));
      } break;
    }
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