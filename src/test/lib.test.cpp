#include "helpers.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

using ss::Value;
using ss::VM;
using ss::VMConfig;

class TestVM: public testing::Test
{
 public:
  void SetUp() override
  {
    this->ostream = std::make_shared<std::ostringstream>();
    this->vm      = std::make_shared<VM>(VMConfig(&std::cin, this->ostream.get()));
  }

 protected:
  std::shared_ptr<std::ostringstream> ostream;
  std::shared_ptr<VM>                 vm;
};

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

TEST_F(TestVM, setting_and_getting_vars)
{
  auto script = R"(
    print value;
    value = true;
  )";

  this->vm->set_var("value", Value("test"));
  this->vm->run_script(script);

  EXPECT_EQ(this->ostream->str(), "test\n");
  EXPECT_EQ(this->vm->get_var("value"), Value(true));
}
