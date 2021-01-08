#include "datatypes.hpp"
#include "exceptions.hpp"
#include <sstream>

namespace ss
{
  Value::Value(): value(nullptr) {}

  Value::Value(NumberType v): value(v) {}

  Value::Value(StringType v): value(v) {}

  auto Value::number() const -> NumberType
  {
    if (this->is_type(Type::Number)) {
      return std::get<NumberType>(this->value);
    } else {
      return 0;
    }
  }

  auto Value::string() const -> std::string
  {
    if (this->is_type(Type::String)) {
      return std::get<StringType>(this->value);
    } else {
      return std::string();
    }
  }

  auto Value::to_string() const -> std::string
  {
    switch (this->type()) {
      case Type::Nil: {
        return std::string("nil");
      }
      case Type::Number: {
        std::stringstream ss;
        ss << std::get<NumberType>(this->value);
        return ss.str();
      }
      case Type::String: {
        return std::get<StringType>(this->value);
      }
      default: {
        // virtually impossible to get here, unable to test
        THROW_RUNTIME_ERROR("tried converting invalid type to string");
      }
    }
  }

  auto Value::operator-() const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        return Value(-std::get<NumberType>(this->value));
      }
      default: {
        THROW_RUNTIME_ERROR("negation on invalid type");
      }
    }
  }

  auto Value::operator+(const Value& other) const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        auto a = std::get<NumberType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            return Value(a + b);
          }
          case Type::String: {
            auto              b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to add invalid types");
          }
        }
      }
      case Type::String: {
        auto a = std::get<StringType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto              b = std::get<NumberType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          case Type::String: {
            auto              b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to add invalid types");
          }
        }
      }
      default: {
        THROW_RUNTIME_ERROR("unable to add invalid types");
      }
    }
  }

  auto Value::operator-(const Value& other) const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        auto a = std::get<NumberType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            return Value(a - b);
          }
          default: {
            THROW_RUNTIME_ERROR("unable to sub invalid types");
          }
        }
      }
      default: {
        THROW_RUNTIME_ERROR("unable to sub invalid types");
      }
    }
  }

  auto Value::operator*(const Value& other) const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        auto a = std::get<NumberType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            return Value(a * b);
          }
          case Type::String: {
            auto              b = std::get<StringType>(other.value);
            std::stringstream ss;
            for (double i = 0; i < a; i++) {
              ss << b;
            }
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to mul invalid types");
          }
        }
      }
      case Type::String: {
        auto a = std::get<StringType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto              b = std::get<NumberType>(other.value);
            std::stringstream ss;
            for (double i = 0; i < b; i++) {
              ss << a;
            }
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to mul invalid types");
          }
        }
      }
      default: {
        THROW_RUNTIME_ERROR("unable to mul invalid types");
      }
    }
  }

  auto Value::operator/(const Value& other) const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        auto a = std::get<NumberType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            return Value(a / b);
          }
          default: {
            THROW_RUNTIME_ERROR("unable to div invalid types");
          }
        }
      }
      default: {
        THROW_RUNTIME_ERROR("unable to div invalid types");
      }
    }
  }

  auto Value::operator=(NumberType v) noexcept -> Value&
  {
    return *this = std::move(Value(v));
  }

  auto Value::operator=(StringType v) noexcept -> Value&
  {
    return *this = std::move(Value(v));
  }

  auto Value::operator==(const Value& other) const noexcept -> bool
  {
    return this->value == other.value;
  }

  auto Value::type() const noexcept -> Type
  {
    return static_cast<Type>(this->value.index());
  }

  auto Value::is_type(Type t) const noexcept -> bool
  {
    return this->type() == t;
  }
}  // namespace ss
