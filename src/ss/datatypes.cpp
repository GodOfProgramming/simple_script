#include "datatypes.hpp"
#include "exceptions.hpp"

namespace ss
{
  Value::Value(NumberType v): datatype(Type::Number), value(v) {}

  Value::Value(StringType v): datatype(Type::String), value(v) {}

  auto Value::number() -> NumberType*
  {
    if (this->datatype == Type::Number) {
      return &std::get<NumberType>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::string() -> std::string*
  {
    if (this->datatype == Type::String) {
      return &std::get<StringType>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::to_string() const -> std::string
  {
    switch (this->datatype) {
      case Type::Number: {
        return std::to_string(std::get<NumberType>(this->value));
      } break;
      case Type::String: {
        return std::get<StringType>(this->value);
      } break;
      default: {
        THROW_RUNTIME_ERROR("this should not be possible");
      }
    }
  }

  auto Value::operator-() -> Value
  {
    switch (this->datatype) {
      case Type::Number: {
        return Value(-std::get<NumberType>(this->value));
      } break;
      default: {
        THROW_RUNTIME_ERROR("negation on invalid type");
      } break;
    }
  }

  auto Value::operator=(NumberType v) -> Value&
  {
    return *this = std::move(Value(v));
  }

  auto Value::operator=(StringType v) -> Value&
  {
    return *this = std::move(Value(v));
  }
}  // namespace ss
