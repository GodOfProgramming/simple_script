#include "ss/exceptions.hpp"
#include "ss/vm.hpp"

int main(int argc, char* argv[])
{
  using ss::CompiletimeError;
  using ss::RuntimeError;
  using ss::VM;

  VM vm;

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