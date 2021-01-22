#include "helpers.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::VM;
using ss::VMConfig;

TEST(VM, prints_correctly)
{
  std::ostringstream oss;
  VMConfig           config(&std::cin, &oss);
  VM                 vm(config);

#define TEST_SCRIPT(src) #src
  const char* script = {
#include "scripts/print_script.ss"
  };
#undef TEST_SCRIPT

  vm.run_script(script);

  ASSERT_EQ(oss.str(), "true\nhello world\n");
}
