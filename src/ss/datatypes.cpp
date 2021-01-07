#include "datatypes.hpp"

namespace ss
{
  Value::Value(double v): datatype(Type::Number), value(v) {}

  Value::Value(std::string v): datatype(Type::String), value(v) {}

  auto Value::number() -> double*
  {
    if (this->datatype == Type::Number) {
      return &std::get<double>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::string() -> std::string*
  {
    if (this->datatype == Type::String) {
      return &std::get<std::string>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::to_string() -> std::string
  {
    switch (this->datatype) {
      case Type::Number: {
        return std::to_string(std::get<double>(this->value));
      } break;
      case Type::String: {
        return std::get<std::string>(this->value);
      } break;
    }
  }

  auto Value::operator=(double v) -> Value&
  {
    return *this = Value(v);
  }

  auto Value::operator=(std::string v) -> Value&
  {
    return *this = Value(v);
  }
}  // namespace ss
