#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace ss
{
  class Function;

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

    using BoolType     = bool;
    using NumberType   = double;
    using StringType   = std::string;
    using FunctionType = std::shared_ptr<Function>;

    Value();
    Value(BoolType v);
    Value(NumberType v);
    Value(StringType v);
    Value(const char* v);
    Value(FunctionType v);

    auto type() const noexcept -> Type;
    auto is_type(Type t) const noexcept -> bool;

    auto boolean() const -> BoolType;
    auto number() const -> NumberType;
    auto string() const -> StringType;
    auto function() const -> FunctionType;

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
    auto operator=(FunctionType) noexcept -> Value&;

    auto operator==(const Value& other) const noexcept -> bool;
    auto operator!=(const Value& other) const noexcept -> bool;
    auto operator>(const Value& other) const noexcept -> bool;
    auto operator>=(const Value& other) const noexcept -> bool;
    auto operator<(const Value& other) const noexcept -> bool;
    auto operator<=(const Value& other) const noexcept -> bool;

    static NilType nil;

   private:
    std::variant<NilType, BoolType, NumberType, StringType, FunctionType> value;
  };

  auto operator<<(std::ostream& ostream, const Value& value) -> std::ostream&;

  class Function
  {
   public:
    Function() noexcept;
    virtual ~Function() = default;

    virtual auto call(std::vector<Value>&& args) -> Value = 0;

    virtual auto to_string() const noexcept -> std::string = 0;

   protected:
    std::string name;
    std::size_t airity;
  };

  auto operator<<(std::ostream& ostream, const Function& fn) -> std::ostream&;

  class ScriptFunction: public Function
  {
   public:
    ~ScriptFunction() override = default;

    auto call(std::vector<Value>&& args) -> Value override;

    auto to_string() const noexcept -> std::string override;

   private:
    std::size_t ip;
  };

  class NativeFunction: public Function
  {
   public:
    ~NativeFunction() override = default;

    virtual auto call(std::vector<Value>&& args) -> Value;

    virtual auto to_string() const noexcept -> std::string;
  };
}  // namespace ss