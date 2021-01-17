#include "datatypes.hpp"
#include "exceptions.hpp"
#include <sstream>
#include <cmath>

namespace ss
{
  Value::NilType Value::nil;

  Value::Value(): value(nil) {}

  Value::Value(BoolType v): value(v) {}

  Value::Value(NumberType v): value(v) {}

  Value::Value(StringType v): value(v) {}

  Value::Value(const char* v): Value(std::string(v)) {}

  auto Value::boolean() const -> BoolType
  {
    if (this->is_type(Type::Bool)) {
      return std::get<BoolType>(this->value);
    } else {
      return BoolType();
    }
  }

  auto Value::number() const -> NumberType
  {
    if (this->is_type(Type::Number)) {
      return std::get<NumberType>(this->value);
    } else {
      return NumberType();
    }
  }

  auto Value::string() const -> std::string
  {
    if (this->is_type(Type::String)) {
      return std::get<StringType>(this->value);
    } else {
      return StringType();
    }
  }

  auto Value::truthy() const -> bool
  {
    switch (this->type()) {
      case Type::Nil: {
        return false;
      }
      case Type::Bool: {
        return this->boolean();
      }
      default: {
        return true;
      }
    }
  }

  auto Value::to_string() const -> std::string
  {
    switch (this->type()) {
      case Type::Nil: {
        return std::string("nil");
      }
      case Type::Bool: {
        if (std::get<BoolType>(this->value)) {
          return std::string("true");
        } else {
          return std::string("false");
        }
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

  auto Value::operator!() const -> Value
  {
    switch (this->type()) {
      case Type::Bool: {
        return Value(!std::get<BoolType>(this->value));
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

  auto Value::operator%(const Value& other) const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        auto a = std::get<NumberType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            return Value(std::fmod(a, b));
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

  auto Value::operator=(NilType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(BoolType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(NumberType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(StringType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(const char* v) noexcept -> Value&
  {
    return *this = StringType(v);
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
