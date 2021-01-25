#include "ss/exceptions.hpp"
#include "ss/vm.hpp"

int main(int argc, char* argv[])
{
  using ss::CompiletimeError;
  using ss::RuntimeError;
  using ss::VM;

  VM vm;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      try {
        vm.run_file(argv[i]);
      } catch (CompiletimeError& e) {
        std::cout << "compile error: " << e.what() << '\n';
      } catch (RuntimeError& e) {
        std::cout << "runtime error: " << e.what() << '\n';
      }
    }
  } else {
    return vm.repl();
  }
}