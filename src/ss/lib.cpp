#include "exceptions.hpp"
#include "lib.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
  constexpr bool DISASSEMBLE_CHUNK = true;
  constexpr bool SHOW_DISASSEMBLY  = false;
  constexpr bool PRINT_STACK       = false;
  constexpr bool ECHO_INPUT        = false;
}  // namespace

namespace ss
{
  VM::VM(VMConfig cfg): config(cfg), state(nullptr) {}

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
    State state;

    compile(src, state);

    this->interpret(state);
  }

  void VM::run_chunk()
  {
    if constexpr (DISASSEMBLE_CHUNK) {
      this->disassemble_chunk("TODO", *this->state);
    }
    while (this->ip < this->state->end()) {
      if constexpr (SHOW_DISASSEMBLY) {
        if constexpr (PRINT_STACK) {
          this->print_stack();
        }
        this->disassemble_instruction(*this->state, *this->ip, this->ip - this->state->begin());
      }
      switch (static_cast<OpCode>(this->ip->major_opcode)) {
        case OpCode::NO_OP:
          break;
        case OpCode::CONSTANT: {
          this->state->push_stack(this->state->constant_at(this->ip->modifying_bits));
        } break;
        case OpCode::NIL: {
          this->state->push_stack(Value());
        } break;
        case OpCode::TRUE: {
          this->state->push_stack(Value(true));
        } break;
        case OpCode::FALSE: {
          this->state->push_stack(Value(false));
        } break;
        case OpCode::POP: {
          this->state->pop_stack();
        } break;
        case OpCode::LOOKUP_GLOBAL: {
          Value name_value = this->state->constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto              var  = this->globals.find(name);
          if (var == this->globals.end()) {
            std::stringstream ss;
            ss << "tried using undefined variable '" << name << '\'';
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->state->push_stack(var->second);
        } break;
        case OpCode::DEFINE_GLOBAL: {
          Value name_value = this->state->constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          if (this->globals.find(name) != this->globals.end()) {
            std::stringstream ss;
            ss << "variable '" << name << "' already defined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->globals.emplace(name, this->state->pop_stack());
        } break;
        case OpCode::EQUAL: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a == b);
        } break;
        case OpCode::NOT_EQUAL: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a != b);
        } break;
        case OpCode::GREATER: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a > b);
        } break;
        case OpCode::GREATER_EQUAL: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a >= b);
        } break;
        case OpCode::LESS: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a < b);
        } break;
        case OpCode::LESS_EQUAL: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a <= b);
        } break;
        case OpCode::ADD: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a + b);
        } break;
        case OpCode::SUB: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a - b);
        } break;
        case OpCode::MUL: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a * b);
        } break;
        case OpCode::DIV: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a / b);
        } break;
        case OpCode::MOD: {
          Value b = this->state->pop_stack();
          Value a = this->state->pop_stack();
          this->state->push_stack(a % b);
        } break;
        case OpCode::NOT: {
          this->state->push_stack(!this->state->pop_stack());
        } break;
        case OpCode::NEGATE: {
          this->state->push_stack(-this->state->pop_stack());
        } break;
        case OpCode::PRINT: {
          config.write_line(state->pop_stack());
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

  void VM::interpret(State& state)
  {
    this->state = &state;
    this->ip    = this->state->begin();

    this->run_chunk();
  }

  void VM::disassemble_chunk(std::string name, State& state) noexcept
  {
    this->config.write_line("== ", name, " ==");

    std::size_t offset = 0;
    for (const auto& i : state) {
      this->disassemble_instruction(state, i, offset++);
    }
  }

  void VM::disassemble_instruction(State& state, Instruction i, std::size_t offset) noexcept
  {
#define SS_SIMPLE_PRINT_CASE(name)         \
  case OpCode::name: {                     \
    this->config.write_line(OpCode::name); \
  } break;

    this->config.write(std::setw(4), std::setfill('0'), offset, ' ');

    if (offset > 0 && state.line_at(offset) == state.line_at(offset - 1)) {
      this->config.write("   | ");
    } else {
      this->config.write(std::setw(4), std::setfill('0'), state.line_at(offset), ' ');
    }

    this->config.reset_ostream();

    switch (i.major_opcode) {
      SS_SIMPLE_PRINT_CASE(NO_OP)
      case OpCode::CONSTANT: {
        Value constant = state.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, OpCode::CONSTANT);
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
        SS_SIMPLE_PRINT_CASE(LOOKUP_GLOBAL)
        SS_SIMPLE_PRINT_CASE(DEFINE_GLOBAL)
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