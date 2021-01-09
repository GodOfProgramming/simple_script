#include "helpers.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::VM;

TEST(VM, METHOD(run, executes_instructions)) {
  VM vm;

  EXPECT_NO_THROW(vm.test());
}
