#include "exceptions.hpp"
#include "lib.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
  constexpr bool SHOW_DISASSEMBLY = true;
}

namespace ss
{
  void VM::test()
  {
    Chunk chunk;
    chunk.write_constant(Value(1), 1);
    chunk.write(Instruction{OpCode::NEGATE}, 1);
    chunk.write_constant(Value("some string"), 1);
    chunk.write(Instruction{OpCode::RETURN}, 2);
    this->interpret(chunk);
  }

  void VM::run()
  {
    while (this->ip < this->chunk->code.end()) {
      if constexpr (SHOW_DISASSEMBLY) {
        this->print_stack();
        this->disassemble_instruction(*this->chunk, *this->ip, this->ip - this->chunk->code.begin());
      }
      switch (static_cast<OpCode>(this->ip->major_opcode)) {
        case OpCode::NO_OP:
          break;
        case OpCode::CONSTANT: {
          this->chunk->push_stack(this->chunk->constant_at(this->ip->modifying_bits));
        } break;
        case OpCode::NEGATE: {
          this->chunk->push_stack(-this->chunk->pop_stack());
        } break;
        case OpCode::RETURN: {
          if (!this->chunk->stack_empty()) {
            std::cout << this->chunk->pop_stack().to_string() << '\n';
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

    this->run();
  }

  void VM::disassemble_chunk(std::string name, Chunk& chunk) const noexcept
  {
    std::cout << "== " << name << " ==\n";

    std::size_t offset = 0;
    for (const auto& i : chunk.code) {
      this->disassemble_instruction(chunk, i, offset++);
    }
  }

  void VM::disassemble_instruction(Chunk& chunk, Instruction i, std::size_t offset) const noexcept
  {
    printf("%04lu ", offset);

    if (offset > 0 && chunk.line_at(offset) == chunk.line_at(offset - 1)) {
      printf("   | ");
    } else {
      printf("%04lu ", chunk.line_at(offset));
    }

    switch (i.major_opcode) {
      case OpCode::NO_OP: {
        std::cout << to_string(OpCode::NO_OP) << '\n';
      } break;
      case OpCode::CONSTANT: {
        Value constant = chunk.constant_at(i.modifying_bits);
        printf("%-16s %4lu '%s'\n", to_string(OpCode::CONSTANT), i.modifying_bits, constant.to_string().c_str());
      } break;
      case OpCode::NEGATE: {
        std::cout << to_string(OpCode::NEGATE) << '\n';
      } break;
      case OpCode::RETURN: {
        std::cout << to_string(OpCode::RETURN) << '\n';
      } break;
      default: {
        std::cout << to_string(i.major_opcode) << ": " << static_cast<std::uint8_t>(i.major_opcode) << '\n';
      } break;
    }
  }

  void VM::print_stack() const noexcept
  {
    std::cout << "          ";
    for (const auto& value : chunk->stack) {
      std::cout << "[ " << value.to_string() << " ]";
    }
    std::cout << '\n';
  }
}  // namespace ss