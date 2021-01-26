#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace ss
{
  class Function;
  class NativeFunction;

  class Value
  {
   public:
    enum class Type
    {
      Nil,
      Bool,
      Number,
      String,
      Function,
      Native,
      Address,
    };

    struct NilType
    {
      constexpr auto operator==(const NilType&) const noexcept -> bool
      {
        return true;
      }

      constexpr auto operator!=(const NilType&) const noexcept -> bool
      {
        return false;
      }

      constexpr auto operator>(const NilType) const noexcept -> bool
      {
        return false;
      }

      constexpr auto operator>=(const NilType) const noexcept -> bool
      {
        return false;
      }

      constexpr auto operator<(const NilType) const noexcept -> bool
      {
        return false;
      }

      constexpr auto operator<=(const NilType) const noexcept -> bool
      {
        return false;
      }
    };

    using BoolType           = bool;
    using NumberType         = double;
    using StringType         = std::string;
    using FunctionType       = std::shared_ptr<Function>;
    using NativeFunctionType = std::shared_ptr<NativeFunction>;

    struct AddressType
    {
      std::size_t ptr;

      auto operator==(const AddressType& other) const noexcept -> bool;

      auto operator!=(const AddressType& other) const noexcept -> bool;

      auto operator>(const AddressType other) const noexcept -> bool;

      auto operator>=(const AddressType other) const noexcept -> bool;

      auto operator<(const AddressType other) const noexcept -> bool;

      auto operator<=(const AddressType other) const noexcept -> bool;
    };

    Value();
    Value(BoolType v);
    Value(NumberType v);
    Value(StringType v);
    Value(const char* v);
    Value(FunctionType v);
    Value(NativeFunctionType v);
    Value(AddressType v);

    auto type() const noexcept -> Type;
    auto is_type(Type t) const noexcept -> bool;

    auto boolean() const -> BoolType;
    auto number() const -> NumberType;
    auto string() const -> StringType;
    auto function() const -> FunctionType;
    auto native() const -> NativeFunctionType;
    auto address() const -> AddressType;

    auto truthy() const -> bool;
    auto to_string() const -> std::string;

    auto operator-() const -> Value;
    auto operator!() const -> Value;

    auto operator+(const Value& other) const -> Value;
    auto operator-(const Value& other) const -> Value;
    auto operator*(const Value& other) const -> Value;
    auto operator/(const Value& other) const -> Value;
    auto operator%(const Value& other) const -> Value;

    auto operator=(NilType v) noexcept -> Value&;
    auto operator=(BoolType b) noexcept -> Value&;
    auto operator=(NumberType v) noexcept -> Value&;
    auto operator=(StringType v) noexcept -> Value&;
    auto operator=(const char* v) noexcept -> Value&;
    auto operator=(FunctionType v) noexcept -> Value&;
    auto operator=(NativeFunctionType v) noexcept -> Value&;

    auto operator==(const Value& other) const noexcept -> bool;
    auto operator!=(const Value& other) const noexcept -> bool;
    auto operator>(const Value& other) const noexcept -> bool;
    auto operator>=(const Value& other) const noexcept -> bool;
    auto operator<(const Value& other) const noexcept -> bool;
    auto operator<=(const Value& other) const noexcept -> bool;

    static NilType nil;

   private:
    std::variant<NilType, BoolType, NumberType, StringType, FunctionType, NativeFunctionType, AddressType> value;
  };

  auto operator<<(std::ostream& ostream, const Value& value) -> std::ostream&;

  class Function
  {
   public:
    Function(std::string name, std::size_t airity, std::size_t ip) noexcept;
    ~Function() = default;

    auto to_string() const noexcept -> std::string;

    const std::string name;
    const std::size_t airity;
    const std::size_t instruction_ptr;
  };

  auto operator<<(std::ostream& ostream, const Function& fn) -> std::ostream&;

  class NativeFunction
  {
   public:
    using Args     = std::vector<Value>;
    using Function = std::function<Value(Args&&)>;

    NativeFunction(std::string name, std::size_t airity, Function function);
    ~NativeFunction() = default;

    auto call(std::vector<Value>&& args) -> Value;

    auto to_string() const noexcept -> std::string;

    const std::string name;
    const std::size_t airity;
    const Function function;
  };
}  // namespace ss