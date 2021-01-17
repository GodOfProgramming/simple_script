#pragma once

#include <string>
#include <variant>

namespace ss
{
  class Value
  {
   public:
    enum class Type
    {
      Nil,
      Bool,
      Number,
      String,
    };

    struct NilType
    {
      constexpr auto operator==(const NilType&) const -> bool
      {
        return true;
      }
    };

    using BoolType = bool;
    using NumberType = double;
    using StringType = std::string;

    Value();
    Value(BoolType v);
    Value(NumberType v);
    Value(StringType v);
    Value(const char* v);

    auto type() const noexcept -> Type;
    auto is_type(Type t) const noexcept -> bool;

    auto boolean() const -> BoolType;
    auto number() const -> NumberType;
    auto string() const -> StringType;

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

    auto operator==(const Value& other) const noexcept -> bool;

    static NilType nil;

   private:
    std::variant<NilType, BoolType, NumberType, StringType> value;
  };
}  // namespace ss