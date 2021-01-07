#include <string>
#include <variant>

namespace ss
{
  class Value
  {
    using Container = std::variant<double, std::string>;
    enum class Type
    {
      Number,
      String,
    };

   public:
    Value(double v);
    Value(std::string v);

    auto number() -> double*;
    auto string() -> std::string*;

    auto to_string() -> std::string;

    auto operator=(double v) -> Value&;
    auto operator=(std::string v) -> Value&;

   private:
    Type      datatype;
    Container value;
  };
}  // namespace ss