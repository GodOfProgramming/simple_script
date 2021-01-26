#include "datatypes.hpp"

#include "exceptions.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace ss
{
  Value::NilType Value::nil;

  Value::Value()
   : value(nil)
  {}

  Value::Value(BoolType v)
   : value(v)
  {}

  Value::Value(NumberType v)
   : value(v)
  {}

  Value::Value(StringType v)
   : value(v)
  {}

  Value::Value(const char* v)
   : Value(std::string(v))
  {}

  Value::Value(FunctionType v)
   : value(v)
  {}

  Value::Value(NativeFunctionType v)
   : value(v)
  {}

  Value::Value(AddressType v)
   : value(v)
  {}

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

  auto Value::string() const -> StringType
  {
    if (this->is_type(Type::String)) {
      return std::get<StringType>(this->value);
    } else {
      return StringType();
    }
  }

  auto Value::function() const -> FunctionType
  {
    if (this->is_type(Type::Function)) {
      return std::get<FunctionType>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::native() const -> NativeFunctionType
  {
    if (this->is_type(Type::Native)) {
      return std::get<NativeFunctionType>(this->value);
    } else {
      return nullptr;
    }
  }

  auto Value::address() const -> AddressType
  {
    if (this->is_type(Type::Address)) {
      return std::get<AddressType>(this->value);
    } else {
      return AddressType{};
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
      default:
        break;
    }

    return true;
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
      case Type::Address: {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << std::get<AddressType>(this->value).ptr;
        return ss.str();
      }
      default:
        break;
    }
    RuntimeError::throw_err("tried converting invalid type to string");
    return std::string();
  }

  auto Value::operator-() const -> Value
  {
    switch (this->type()) {
      case Type::Number: {
        return Value(-std::get<NumberType>(this->value));
      }
      default:
        break;
    }
    RuntimeError::throw_err("negation on invalid type");
    return Value();
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
            auto b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          default:
            break;
        }
      } break;
      case Type::String: {
        auto a = std::get<StringType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          case Type::String: {
            auto b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << a << b;
            return Value(ss.str());
          }
          case Type::Bool: {
            auto b = std::get<BoolType>(other.value);
            std::stringstream ss;
            ss << a << (b ? "true" : "false");
            return Value(ss.str());
          }
          default:
            break;
        }
      } break;
      case Type::Bool: {
        auto a = std::get<BoolType>(this->value);
        switch (other.type()) {
          case Type::String: {
            auto b = std::get<StringType>(other.value);
            std::stringstream ss;
            ss << (a ? "true" : "false") << b;
            return Value(ss.str());
          }
          default:
            break;
        }
      } break;
      default:
        break;
    }
    RuntimeError::throw_err("unable to add invalid types");
    return Value();
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
          default:
            break;
        }
      }
      default:
        break;
    }
    RuntimeError::throw_err("unable to sub invalid types");
    return Value();
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
            auto b = std::get<StringType>(other.value);
            std::stringstream ss;
            for (double i = 0; i < a; i++) { ss << b; }
            return Value(ss.str());
          }
          default:
            break;
        }
      } break;
      case Type::String: {
        auto a = std::get<StringType>(this->value);
        switch (other.type()) {
          case Type::Number: {
            auto b = std::get<NumberType>(other.value);
            std::stringstream ss;
            for (double i = 0; i < b; i++) { ss << a; }
            return Value(ss.str());
          }
          default:
            break;
        }
      }
      default:
        break;
    }
    RuntimeError::throw_err("unable to mul invalid types");
    return Value();
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
          default:
            break;
        }
      }
      default:
        break;
    }
    RuntimeError::throw_err("unable to div invalid types");
    return Value();
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
          default:
            break;
        }
      }
      default:
        break;
    }
    RuntimeError::throw_err("unable to mod invalid types");
    return Value();
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

  auto Value::operator=(FunctionType v) noexcept -> Value&
  {
    this->value = v;
    return *this;
  }

  auto Value::operator=(NativeFunctionType v) noexcept -> Value&
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

  Function::Function(std::string n, std::size_t a, std::size_t ip) noexcept
   : name(n)
   , airity(a)
   , instruction_ptr(ip)
  {}

  auto Function::to_string() const noexcept -> std::string
  {
    std::stringstream ss;
    ss << "<fn " << this->name << '>';
    return ss.str();
  }

  auto operator<<(std::ostream& ostream, const Function& fn) -> std::ostream&
  {
    return ostream << fn.to_string();
  }

  NativeFunction::NativeFunction(std::string n, std::size_t a, Function f)
   : name(n)
   , airity(a)
   , function(f)
  {}

  auto NativeFunction::call(std::vector<Value>&& args) -> Value
  {
    return function(std::move(args));
  }

  auto NativeFunction::to_string() const noexcept -> std::string
  {
    std::stringstream ss;
    ss << "<nf " << this->name << '>';
    return ss.str();
  }

  auto Value::AddressType::operator==(const AddressType& other) const noexcept -> bool
  {
    return this->ptr == other.ptr;
  }

  auto Value::AddressType::operator!=(const AddressType& other) const noexcept -> bool
  {
    return this->ptr != other.ptr;
  }

  auto Value::AddressType::operator>(const AddressType other) const noexcept -> bool
  {
    return this->ptr > other.ptr;
  }

  auto Value::AddressType::operator>=(const AddressType other) const noexcept -> bool
  {
    return this->ptr >= other.ptr;
  }

  auto Value::AddressType::operator<(const AddressType other) const noexcept -> bool
  {
    return this->ptr < other.ptr;
  }

  auto Value::AddressType::operator<=(const AddressType other) const noexcept -> bool
  {
    return this->ptr <= other.ptr;
  }
}  // namespace ss
