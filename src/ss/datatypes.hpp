#pragma once

#include <string>
#include <variant>

namespace ss
{
  class Value
  {
    enum class Type
    {
      Nil,
      Number,
      String,
    };

   public:
    struct NilType
    {
      constexpr auto operator==(const NilType&) const -> bool
      {
        return true;
      }
    };
    using NumberType = double;
    using StringType = std::string;

    Value();
    Value(double v);
    Value(std::string v);

    auto type() const noexcept -> Type;
    auto is_type(Type t) const noexcept -> bool;

    auto number() const -> NumberType;
    auto string() const -> StringType;

    auto to_string() const -> std::string;

    auto operator-() const -> Value;

    auto operator+(const Value& other) const -> Value;
    auto operator-(const Value& other) const -> Value;
    auto operator*(const Value& other) const -> Value;
    auto operator/(const Value& other) const -> Value;

    auto operator=(NilType v) noexcept -> Value&;
    auto operator=(NumberType v) noexcept -> Value&;
    auto operator=(StringType v) noexcept -> Value&;

    auto operator==(const Value& other) const noexcept -> bool;

    static NilType nil;

   private:
    std::variant<NilType, NumberType, StringType> value;
  };
}  // namespace ss