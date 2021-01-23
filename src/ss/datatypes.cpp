#include "datatypes.hpp"

#include "exceptions.hpp"

#include <sstream>
#include <cmath>

namespace ss
{
  Value::NilType Value::nil;

  Value::Value(): value(nil) {}

  Value::Value(BoolType v): value(v) {}

  Value::Value(IntType v): value(v) {}

  Value::Value(UintType v): value(v) {}

  Value::Value(NumberType v): value(v) {}

  Value::Value(StringType v): value(v) {}

  Value::Value(const char* v): Value(std::string(v)) {}

  Value::Value(FunctionType v): value(v) {}

  auto Value::as_bool() const -> BoolType
  {
    if (this->is_type(Type::Bool)) {
      return std::get<BoolType>(this->value);
    } else {
      return BoolType();
    }
  }

  auto Value::as_int() const -> IntType
  {
    if (this->is_type(Type::Int)) {
      return std::get<IntType>(this->value);
    } else {
      return IntType();
    }
  }

  auto Value::as_uint() const -> UintType
  {
    if (this->is_type(Type::Uint)) {
      return std::get<UintType>(this->value);
    } else {
      return BoolType();
    }
  }

  auto Value::as_number() const -> NumberType
  {
    if (this->is_type(Type::Number)) {
      return std::get<NumberType>(this->value);
    } else {
      return NumberType();
    }
  }

  auto Value::as_string() const -> StringType
  {
    if (this->is_type(Type::String)) {
      return std::get<StringType>(this->value);
    } else {
      return StringType();
    }
  }

  auto Value::as_function() const -> FunctionType
  {
    if (this->is_type(Type::Function)) {
      return std::get<FunctionType>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::truthy() const -> bool
  {
    switch (this->type()) {
      case Type::Nil: {
        return false;
      }
      case Type::Bool: {
        return this->as_bool();
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
      case Type::Function: {
        return std::get<FunctionType>(this->value)->to_string();
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
      case Type::Int: {
        return Value(-std::get<IntType>(this->value));
      }
      case Type::Uint: {
        return Value(-IntType{std::get<UintType>(this->value)});
      }
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
    return Value(!this->truthy());
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
          case Type::Bool: {
            auto              b = std::get<BoolType>(other.value);
            std::stringstream ss;
            ss << a << (b ? "true" : "false");
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to add invalid types");
          }
        }
      }
      case Type::Bool: {
        auto a = std::get<BoolType>(this->value);
        switch (other.type()) {
          case Type::String: {
            auto              b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << (a ? "true" : "false") << b;
            return Value(ss.str());
          }
          default: {
            THROW_RUNTIME_ERROR("unable to add invalid types");
          }
        }
      } break;
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
            for (double i = 0; i < a; i++) { ss << b; }
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
            for (double i = 0; i < b; i++) { ss << a; }
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

  auto Value::operator=(IntType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(UintType v) noexcept -> Value&
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

  auto Value::operator=(FunctionType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator==(const Value& other) const noexcept -> bool
  {
    return this->value == other.value;
  }

  auto Value::operator!=(const Value& other) const noexcept -> bool
  {
    return this->value != other.value;
  }

  auto Value::operator>(const Value& other) const noexcept -> bool
  {
    return this->value > other.value;
  }

  auto Value::operator>=(const Value& other) const noexcept -> bool
  {
    return this->value >= other.value;
  }

  auto Value::operator<(const Value& other) const noexcept -> bool
  {
    return this->value < other.value;
  }

  auto Value::operator<=(const Value& other) const noexcept -> bool
  {
    return this->value < other.value;
  }

  auto Value::type() const noexcept -> Type
  {
    return static_cast<Type>(this->value.index());
  }

  auto Value::is_type(Type t) const noexcept -> bool
  {
    return this->type() == t;
  }

  auto operator<<(std::ostream& ostream, const Value& value) -> std::ostream&
  {
    return ostream << value.to_string();
  }

  Function::Function() noexcept: airity(0) {}

  auto Function::to_string() const noexcept -> std::string
  {
    return this->name;
  }

  auto operator<<(std::ostream& ostream, const Function& fn) -> std::ostream&
  {
    return ostream << fn.to_string();
  }

  auto ScriptFunction::call(std::vector<Value>&& args) -> Value
  {
    (void)args;
    return Value();
  }

  auto ScriptFunction::to_string() const noexcept -> std::string
  {
    std::stringstream ss;
    ss << "<fn " << this->name << '>';
    return ss.str();
  }

  auto NativeFunction::call(std::vector<Value>&& args) -> Value
  {
    (void)args;
    return Value();
  }

  auto NativeFunction::to_string() const noexcept -> std::string
  {
    std::stringstream ss;
    ss << "<nf " << this->name << '>';
    return ss.str();
  }
}  // namespace ss
