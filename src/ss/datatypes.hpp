#pragma once

#include <string>
#include <variant>

namespace ss
{
  class Value
  {
    enum class Type
    {
      Number,
      String,
    };

   public:
    using NumberType = double;
    using StringType = std::string;

    Value(double v);
    Value(std::string v);

    auto number() -> NumberType*;
    auto string() -> StringType*;

    auto to_string() const -> std::string;

    auto operator-() -> Value;

    auto operator=(NumberType v) -> Value&;
    auto operator=(StringType v) -> Value&;

   private:
    using Container = std::variant<NumberType, StringType>;

    Type      datatype;
    Container value;
  };
}  // namespace ss