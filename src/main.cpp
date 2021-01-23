#include "ss/lib.hpp"
#include "ss/exceptions.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
  using ss::CompiletimeError;
  using ss::RuntimeError;
  using ss::VM;

  VM vm;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      std::string                            contents;
      std::ifstream                          istr(argv[i]);
      std::istreambuf_iterator<char>         input_iter(istr), empty_iter;
      std::back_insert_iterator<std::string> string_inserter(contents);
      std::copy(input_iter, empty_iter, string_inserter);
      try {
        vm.run_script(contents);
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