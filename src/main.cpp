#include "ss/exceptions.hpp"
#include "ss/vm.hpp"

#include <chrono>

int main(int argc, char* argv[])
{
  using ss::CompiletimeError;
  using ss::NativeFunction;
  using ss::RuntimeError;
  using ss::Value;
  using ss::VM;
  using Args = ss::NativeFunction::Args;

  VM vm;

  vm.set_var("clock", Value(std::make_shared<NativeFunction>("clock", 0, [](Args&&) {
               auto tp                                       = std::chrono::high_resolution_clock::now();
               std::chrono::duration<Value::NumberType> secs = tp.time_since_epoch();
               return Value(Value::NumberType{secs.count()});
             })));

  if (argc > 1) {
    try {
      auto ret = vm.run_file(argv[1]);
      if (ret.is_type(ss::Value::Type::Number)) {
        std::cout << "got " << ret << '\n';
        return static_cast<int>(ret.number());
      } else {
        return 0;
      }
    } catch (CompiletimeError& e) {
      std::cout << "compile error: " << e.what() << '\n';
      return 1;
    } catch (RuntimeError& e) {
      std::cout << "runtime error: " << e.what() << '\n';
      return 1;
    } catch (std::exception& e) {
      std::cout << "exception: " << e.what() << '\n';
      return 1;
    }
  } else {
    return vm.repl();
  }
}