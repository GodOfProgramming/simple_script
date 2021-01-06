#include "ss/lib.hpp"
#include <iostream>

namespace ss
{
  auto VM::run() -> InterpretResult
  {
    while (ip < this->chunk->code.end()) {
      switch (static_cast<OpCode>(*this->ip)) {
        case OpCode::RETURN: {
          return InterpretResult::OK;
        }
      }
      this->ip++;
    }

    return InterpretResult::OK;
  }

  auto VM::interpret(Chunk* chunk) -> InterpretResult
  {
    this->chunk = chunk;
    this->ip    = chunk->code.begin();

    this->run();
  }
}  // namespace ss