#include "lib.hpp"

#include "exceptions.hpp"
#include "util.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ss
{
  VM::VM(VMConfig cfg)
   : config(cfg)
   , sp(0)
  {}

  void VM::set_var(Value::StringType name, Value value) noexcept
  {
    this->chunk.set_global(std::move(name), value);
  }

  auto VM::get_var(Value::StringType name) noexcept -> Value
  {
    return this->chunk.find_global(name)->second;
  }

  auto VM::repl(VMConfig cfg) -> int
  {
    VM vm(cfg);
    bool exit               = false;
    int exit_code           = 0;
    std::size_t line_number = 1;

    while (!exit) {
      std::string line;
      cfg.write("ss(main):", line_number, "> ");
      cfg.read_line(line);

      if (ECHO_INPUT) {
        cfg.write_line(line);
      }

      try {
        vm.run_line(line);
        line_number++;
      } catch (CompiletimeError& e) {
        cfg.write_line("compile error: ", e.what());
      } catch (RuntimeError& e) {
        cfg.write_line("runtime error: ", e.what());
      }
    }

    return exit_code;
  }

  void VM::run_file(std::string filename)
  {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::stringstream ss;
    ss << cwd.string() << '/' << filename;
    this->run_script(util::load_file_to_string(filename), ss.str());
  }

  void VM::run_script(std::string src, std::filesystem::path path)
  {
    this->chunk.prepare();
    this->compile(path.string(), std::move(src));
    this->ip = this->chunk.begin();
    this->execute();
  }

  void VM::run_line(std::string line)
  {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::size_t offset        = this->chunk.instruction_count();
    this->compile(cwd.string(), std::move(line));
    this->ip = this->chunk.begin() + offset;
    this->execute();
  }

  void VM::compile(std::string filename, std::string&& src)
  {
    Compiler compiler;

    compiler.compile(std::move(src), this->chunk, filename);
  }

  void VM::execute()
  {
    if constexpr (DISASSEMBLE_CHUNK) {
      this->disassemble_chunk();
    }
    if constexpr (PRINT_CONSTANTS) {
      this->chunk.print_constants(this->config);
    }
    if constexpr (PRINT_LOCAL_MAPPING) {
      this->chunk.print_local_map(this->config);
    }
    while (this->ip < this->chunk.end()) {
      if constexpr (DISASSEMBLE_INSTRUCTIONS) {
        if constexpr (PRINT_STACK) {
          this->chunk.print_stack(this->config);
        }
        this->disassemble_instruction(*this->ip, this->ip - this->chunk.begin());
      }

      switch (this->ip->major_opcode) {
        case OpCode::NO_OP:
          break;
        case OpCode::CONSTANT: {
          this->chunk.push_stack(this->chunk.constant_at(this->ip->modifying_bits));
        } break;
        case OpCode::NIL: {
          this->chunk.push_stack(Value());
        } break;
        case OpCode::TRUE: {
          this->chunk.push_stack(Value(true));
        } break;
        case OpCode::FALSE: {
          this->chunk.push_stack(Value(false));
        } break;
        case OpCode::POP: {
          this->chunk.pop_stack();
        } break;
        case OpCode::POP_N: {
          this->chunk.pop_stack_n(this->ip->modifying_bits);
        } break;
        case OpCode::LOOKUP_LOCAL: {
          this->chunk.push_stack(this->chunk.index_stack(this->ip->modifying_bits));
        } break;
        case OpCode::ASSIGN_LOCAL: {
          this->chunk.index_stack_mut(this->ip->modifying_bits) = this->chunk.peek_stack();
        } break;
        case OpCode::LOOKUP_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(name);
          if (!this->chunk.is_global_found(var)) {
            std::stringstream ss;
            ss << "variable '" << name << "' is undefined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->chunk.push_stack(var->second);
        } break;
        case OpCode::DEFINE_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(name);
          if (this->chunk.is_global_found(var)) {
            std::stringstream ss;
            ss << "variable '" << name << "' is already defined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          this->chunk.set_global(std::move(name), this->chunk.pop_stack());
        } break;
        case OpCode::ASSIGN_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            THROW_RUNTIME_ERROR("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(std::move(name));
          if (!this->chunk.is_global_found(var)) {
            std::stringstream ss;
            ss << "variable '" << name << "' is undefined";
            THROW_RUNTIME_ERROR(ss.str());
          }
          var->second = this->chunk.peek_stack();
        } break;
        case OpCode::EQUAL: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a == b);
        } break;
        case OpCode::NOT_EQUAL: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a != b);
        } break;
        case OpCode::GREATER: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a > b);
        } break;
        case OpCode::GREATER_EQUAL: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a >= b);
        } break;
        case OpCode::LESS: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a < b);
        } break;
        case OpCode::LESS_EQUAL: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a <= b);
        } break;
        case OpCode::CHECK: {
          Value v = this->chunk.pop_stack();
          this->chunk.push_stack(this->chunk.peek_stack() == v);
        } break;
        case OpCode::ADD: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a + b);
        } break;
        case OpCode::SUB: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a - b);
        } break;
        case OpCode::MUL: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a * b);
        } break;
        case OpCode::DIV: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a / b);
        } break;
        case OpCode::MOD: {
          Value b = this->chunk.pop_stack();
          Value a = this->chunk.pop_stack();
          this->chunk.push_stack(a % b);
        } break;
        case OpCode::NOT: {
          this->chunk.push_stack(!this->chunk.pop_stack());
        } break;
        case OpCode::NEGATE: {
          this->chunk.push_stack(-this->chunk.pop_stack());
        } break;
        case OpCode::PRINT: {
          config.write_line(this->chunk.pop_stack());
        } break;
        case OpCode::JUMP: {
          this->ip += this->ip->modifying_bits;
          continue;
        } break;
        case OpCode::JUMP_IF_FALSE: {
          if (!this->chunk.peek_stack().truthy()) {
            this->ip += this->ip->modifying_bits;
            continue;
          }
        } break;
        case OpCode::LOOP: {
          this->ip -= this->ip->modifying_bits;
          continue;
        } break;
        case OpCode::OR: {
          Value v = this->chunk.peek_stack();
          if (v.truthy()) {
            this->ip += this->ip->modifying_bits;
            continue;
          } else {
            this->chunk.pop_stack();
          }
        } break;
        case OpCode::AND: {
          Value v = this->chunk.peek_stack();
          if (!v.truthy()) {
            this->ip += this->ip->modifying_bits;
            continue;
          } else {
            this->chunk.pop_stack();
          }
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
    if constexpr (PRINT_STACK) {
      this->chunk.print_stack(this->config);
    }
  }

  void VM::disassemble_chunk() noexcept
  {
    this->config.write_line("<< ", "MAIN", " >>");
    std::size_t offset = 0;
    for (const auto& i : chunk) { this->disassemble_instruction(i, offset++); }
    this->config.write_line("<< ", "END", " >>");
  }

  void VM::disassemble_instruction(Instruction i, std::size_t offset) noexcept
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
        this->config.write(std::setw(16), std::left, i.major_opcode);
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
      SS_COMPLEX_PRINT_CASE(POP_N, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(LOOKUP_LOCAL, {
        auto name = chunk.lookup_local(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", name, "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_LOCAL, {
        auto name = chunk.lookup_local(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", name, "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(LOOKUP_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(DEFINE_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write(" '", constant.to_string(), "'\n");
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_GLOBAL, {
        Value constant = chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
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
      SS_SIMPLE_PRINT_CASE(CHECK)
      SS_SIMPLE_PRINT_CASE(ADD)
      SS_SIMPLE_PRINT_CASE(SUB)
      SS_SIMPLE_PRINT_CASE(MUL)
      SS_SIMPLE_PRINT_CASE(DIV)
      SS_SIMPLE_PRINT_CASE(MOD)
      SS_SIMPLE_PRINT_CASE(NOT)
      SS_SIMPLE_PRINT_CASE(NEGATE)
      SS_SIMPLE_PRINT_CASE(PRINT)
      SS_COMPLEX_PRINT_CASE(JUMP, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(JUMP_IF_FALSE, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(LOOP, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(OR, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(AND, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_SIMPLE_PRINT_CASE(RETURN)
      default: {
        this->config.write_line(i.major_opcode, ": ", i.modifying_bits);
      } break;
    }

#undef SS_SIMPLE_PRINT_CASE
  }
}  // namespace ss