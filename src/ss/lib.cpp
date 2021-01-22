#include "exceptions.hpp"
#include "lib.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ss
{
  VM::VM(VMConfig cfg): config(cfg), chunk(nullptr) {}

  void VM::set_var(Value::StringType name, Value value) noexcept
  {
    this->globals[name] = value;
  }

  auto VM::get_var(Value::StringType name) noexcept -> Value
  {
    return this->globals[name];
  }

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
    Compiler compiler;

    BytecodeChunk chunk;
    compiler.compile(src, chunk);

    this->interpret(chunk);
  }

  void VM::run_chunk()
  {
    if constexpr (DISASSEMBLE_CHUNK) {
      this->disassemble_chunk("TODO", *this->chunk);
    }
    if constexpr (PRINT_CONSTANTS) {
      this->chunk->print_constants(this->config);
    }
    if constexpr (PRINT_LOCAL_MAPPING) {
      this->chunk->print_local_map(this->config);
    }
    while (this->ip < this->chunk->end()) {
      if constexpr (DISASSEMBLE_INSTRUCTIONS) {
        if constexpr (PRINT_STACK) {
          this->chunk->print_stack(this->config);
        }
        this->disassemble_instruction(*this->chunk, *this->ip, this->ip - this->chunk->begin());
      }

      switch (this->ip->major_opcode) {
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
        case OpCode::POP_N: {
          this->chunk->pop_stack_n(this->ip->modifying_bits);
        } break;
        case OpCode::LOOKUP_LOCAL: {
          this->chunk->push_stack(this->chunk->index_stack(this->ip->modifying_bits));
        } break;
        case OpCode::ASSIGN_LOCAL: {
          this->chunk->index_stack_mut(this->ip->modifying_bits) = this->chunk->peek_stack();
        } break;
        case OpCode::LOOKUP_GLOBAL: {
          Value name_value = this->chunk->constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto              var  = this->globals.find(name);
          if (var == this->globals.end()) {
            std::stringstream ss;
            ss << "variable '" << name << "' is undefined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->chunk->push_stack(var->second);
        } break;
        case OpCode::DEFINE_GLOBAL: {
          Value name_value = this->chunk->constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          if (this->globals.find(name) != this->globals.end()) {
            std::stringstream ss;
            ss << "variable '" << name << "' is already defined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->globals.emplace(name, this->chunk->pop_stack());
        } break;
        case OpCode::ASSIGN_GLOBAL: {
          Value name_value = this->chunk->constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          if (this->globals.find(name) == this->globals.end()) {
            std::stringstream ss;
            ss << "variable '" << name << "' is undefined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->globals[name] = this->chunk->peek_stack();
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
          ss << "invalid op code: " << static_cast<std::size_t>(this->ip->major_opcode);
          THROW_RUNTIME_ERROR(ss.str());
        }
      }
      this->ip++;
    }
  }

  void VM::interpret(BytecodeChunk& chunk)
  {
    this->chunk = &chunk;
    this->ip    = this->chunk->begin();

    this->run_chunk();
  }

  void VM::disassemble_chunk(std::string name, BytecodeChunk& chunk) noexcept
  {
    this->config.write_line("== ", name, " ==");

    std::size_t offset = 0;
    for (const auto& i : chunk) {
      this->disassemble_instruction(chunk, i, offset++);
    }
  }

  void VM::disassemble_instruction(BytecodeChunk& chunk, Instruction i, std::size_t offset) noexcept
  {
#define SS_SIMPLE_PRINT_CASE(name)                                                                                             \
  case OpCode::name: {                                                                                                         \
    this->config.write_line(OpCode::name);                                                                                     \
  } break;

#define SS_COMPLEX_PRINT_CASE(name, block)                                                                                     \
  case OpCode::name:                                                                                                           \
    block break;

    this->config.write(std::setw(4), std::setfill('0'), offset, ' ');

    if (offset > 0 && chunk.line_at(offset) == chunk.line_at(offset - 1)) {
      this->config.write("   | ");
    } else {
      this->config.write(std::setw(4), std::setfill('0'), chunk.line_at(offset), ' ');
    }

    this->config.reset_ostream();

    switch (i.major_opcode) {
      SS_SIMPLE_PRINT_CASE(NO_OP)
      SS_COMPLEX_PRINT_CASE(CONSTANT, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::CONSTANT);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
      SS_SIMPLE_PRINT_CASE(NIL)
      SS_SIMPLE_PRINT_CASE(TRUE)
      SS_SIMPLE_PRINT_CASE(FALSE)
      SS_SIMPLE_PRINT_CASE(POP)
      SS_SIMPLE_PRINT_CASE(POP_N)
      SS_COMPLEX_PRINT_CASE(LOOKUP_LOCAL, {
        auto name = chunk.lookup_local(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::LOOKUP_LOCAL);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", name, "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_LOCAL, {
        auto name = chunk.lookup_local(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::ASSIGN_LOCAL);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", name, "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(LOOKUP_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::LOOKUP_GLOBAL);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(DEFINE_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::DEFINE_GLOBAL);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::DEFINE_GLOBAL);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
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
        this->config.write_line(i.major_opcode, ": ", i.modifying_bits);
      } break;
    }

#undef SS_SIMPLE_PRINT_CASE
  }
}  // namespace ss