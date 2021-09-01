#include "vm.hpp"

#include "exceptions.hpp"
#include "util.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define SS_SIMPLE_PRINT_CASE(name)                                                                                             \
  case OpCode::name: {                                                                                                         \
    this->config.write_line(OpCode::name);                                                                                     \
  } break;

#define SS_COMPLEX_PRINT_CASE(name, block)                                                                                     \
  case OpCode::name:                                                                                                           \
    block break;

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

  auto VM::run_file(std::string filename) -> Value
  {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::stringstream ss;
    ss << cwd.string() << '/' << filename;
    std::ifstream ifs(filename);
    return this->run_script(util::stream_to_string(ifs), ss.str());
  }

  auto VM::run_script(std::string src, std::filesystem::path path) -> Value
  {
    this->chunk.prepare();
    this->compile(path.string(), std::move(src));
    this->ip = this->chunk.begin();
    return this->execute();
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

  auto VM::execute() -> Value
  {
    if constexpr (DISASSEMBLE_CHUNK) {
      this->disassemble_chunk();
    }
    if constexpr (PRINT_CONSTANTS) {
      this->chunk.print_constants(this->config);
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
          this->chunk.push_stack(this->chunk.index_stack(this->sp + this->ip->modifying_bits));
        } break;
        case OpCode::ASSIGN_LOCAL: {
          this->chunk.index_stack_mut(this->sp + this->ip->modifying_bits) = this->chunk.peek_stack();
        } break;
        case OpCode::LOOKUP_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            RuntimeError::throw_err("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(name);
          if (!this->chunk.is_global_found(var)) {
            RuntimeError::throw_err("variable '", name, "' is undefined");
          }
          this->chunk.push_stack(var->second);
        } break;
        case OpCode::DEFINE_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            RuntimeError::throw_err("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(name);
          if (this->chunk.is_global_found(var)) {
            RuntimeError::throw_err("variable '", name, "' is already defined");
          }
          this->chunk.set_global(std::move(name), this->chunk.pop_stack());
        } break;
        case OpCode::ASSIGN_GLOBAL: {
          Value name_value = this->chunk.constant_at(this->ip->modifying_bits);
          if (!name_value.is_type(Value::Type::String)) {
            RuntimeError::throw_err("invalid type for variable name");
          }
          Value::StringType name = name_value.string();
          auto var               = this->chunk.find_global(std::move(name));
          if (!this->chunk.is_global_found(var)) {
            RuntimeError::throw_err("variable '", name, "' is undefined");
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
        case OpCode::SWAP: {
          Value a = this->chunk.pop_stack();
          Value b = this->chunk.pop_stack();
          this->chunk.push_stack(a);
          this->chunk.push_stack(b);
        } break;
        case OpCode::MOVE: {
          Value top = this->chunk.peek_stack();
          // shift the value down, useful for returning
          this->chunk.index_stack_mut(this->chunk.stack_size() - 1 - this->ip->modifying_bits) = top;
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
        case OpCode::PUSH_SP: {
          this->chunk.push_stack(Value{Value::AddressType{this->sp}});
          // - 1 for the fn on the stack, - 1 because size()
          this->sp = this->chunk.stack_size() - this->ip->modifying_bits - 1 - 1;
        } break;
        case OpCode::CALL: {
          auto fn_val = this->chunk.peek_stack(this->ip->modifying_bits + 2);
          switch (fn_val.type()) {
            case Value::Type::Function: {
              auto fn = fn_val.function();
              if (this->ip->modifying_bits != fn->airity) {
                RuntimeError::throw_err(
                 "tried calling function with incorrect number of args, expected ",
                 fn->airity,
                 ", got ",
                 this->ip->modifying_bits);
              }
              this->ip = this->chunk.index_code_mut(fn->instruction_ptr);
            } break;
            case Value::Type::Native: {
              auto fn = fn_val.native();
              if (this->ip->modifying_bits != fn->airity) {
                RuntimeError::throw_err(
                 "tried calling function with incorrect number of args, expected ",
                 fn->airity,
                 ", got ",
                 this->ip->modifying_bits);
              }
              std::vector<Value> args;
              // remove the stack pointer & return address
              this->chunk.pop_stack_n(2);
              // push arguments into vector
              for (std::size_t i = 0; i < fn->airity; i++) { args.push_back(std::move(this->chunk.pop_stack())); }
              // remove the function
              this->chunk.pop_stack();
              this->chunk.push_stack(fn->call(std::move(args)));
            } break;
            default: {
              RuntimeError::throw_err("tried calling non-function: ", fn_val);
            }
          }
        } break;
        case OpCode::RETURN: {
          auto local_count = this->ip->modifying_bits;
          auto retval      = this->chunk.pop_stack();

          // get the return address
          auto v = this->chunk.pop_stack();
          if (!v.is_type(Value::Type::Address)) {
            RuntimeError::throw_err("trying to return to an invalid value: ", v);
          }
          this->ip = this->chunk.index_code_mut(v.address().ptr);

          // restore the stack pointer
          v        = this->chunk.pop_stack();
          this->sp = v.address().ptr;
          if (!v.is_type(Value::Type::Address)) {
            RuntimeError::throw_err("trying to set the stack pointer to an invalid value: ", v);
          }

          // remove the locals & function
          this->chunk.pop_stack_n(local_count + 1);
          this->chunk.push_stack(retval);
          continue;
        } break;
        case OpCode::END: {
          if constexpr (PRINT_STACK) {
            this->chunk.print_stack(this->config);
          }
          Value retval;
          if (!this->chunk.stack_empty()) {
            retval = this->chunk.pop_stack();
          }
          return retval;
        } break;
        default: {
          RuntimeError::throw_err("invalid op code: ", static_cast<std::size_t>(this->ip->major_opcode));
        }
      }
      this->ip++;
    }

    // never gets here
    return Value();
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
    this->config.write("0x", std::hex, std::setw(4), std::setfill('0'), offset, ' ');
    this->config.reset_ostream();

    if (offset > 0 && this->chunk.line_at(offset) == this->chunk.line_at(offset - 1)) {
      this->config.write("   | ");
    } else {
      this->config.write(std::setw(4), std::setfill('0'), this->chunk.line_at(offset), ' ');
    }

    this->config.reset_ostream();

    switch (i.major_opcode) {
      SS_SIMPLE_PRINT_CASE(NO_OP)
      SS_COMPLEX_PRINT_CASE(CONSTANT, {
        Value constant = this->chunk.constant_at(i.modifying_bits);
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
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write_line(" ", this->sp + i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_LOCAL, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write_line(" ", this->sp + i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(LOOKUP_GLOBAL, {
        Value constant = this->chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write_line(" '", constant.to_string(), '\'');
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(DEFINE_GLOBAL, {
        Value constant = this->chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write_line(" '", constant.to_string(), '\'');
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(ASSIGN_GLOBAL, {
        Value constant = this->chunk.constant_at(i.modifying_bits);
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
        this->config.write_line(" '", constant.to_string(), '\'');
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
      SS_SIMPLE_PRINT_CASE(SWAP)
      SS_COMPLEX_PRINT_CASE(MOVE, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
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
      SS_SIMPLE_PRINT_CASE(PUSH_SP)
      SS_COMPLEX_PRINT_CASE(CALL, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(std::hex, ' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_COMPLEX_PRINT_CASE(RETURN, {
        this->config.write(std::setw(16), std::left, i.major_opcode);
        this->config.reset_ostream();
        this->config.write_line(std::hex, ' ', std::setw(4), i.modifying_bits);
        this->config.reset_ostream();
      })
      SS_SIMPLE_PRINT_CASE(END)
      default: {
        this->config.write_line(i.major_opcode, ": ", i.modifying_bits);
      } break;
    }
  }
}  // namespace ss