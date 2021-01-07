#include <string>
#include <sstream>
#include <iostream>

namespace ss
{
  enum class OpCode
  {
    CONSTANT,
    RETURN,
  };

  class InterpretResult
  {
   public:
    enum class Type
    {
      OK,
      COMPILE_ERROR,
      RUNTIME_ERROR,
    };

    InterpretResult(Type res, std::string msg);

    static auto ok() -> InterpretResult;

    auto is_ok() const -> bool;

    auto to_string() const -> std::string;

   private:
    Type        result;
    std::string error;
  };

  inline InterpretResult::InterpretResult(Type res, std::string msg): result(res), error(msg) {}

  inline auto InterpretResult::ok() -> InterpretResult
  {
    return InterpretResult{Type::OK, std::string()};
  }

  inline auto InterpretResult::is_ok() const -> bool
  {
    this->result == Type::OK;
  }

  inline auto InterpretResult::to_string() const -> std::string
  {
    std::stringstream ss;
    switch (this->result) {
      case Type::OK: {
        ss << "OK: ";
      } break;
      case Type::COMPILE_ERROR: {
        ss << "COMPILE ERROR: ";
      } break;
      case Type::RUNTIME_ERROR: {
        ss << "RUNTIME ERROR: ";
      } break;
    }

    ss << this->error;
    return ss.str();
  }

  std::ostream& operator<<(std::ostream& stream, InterpretResult& err)
  {
    return stream << err.to_string();
  }
}  // namespace ss