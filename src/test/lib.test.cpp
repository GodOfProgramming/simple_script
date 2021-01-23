#include "helpers.hpp"
#include "ss/lib.hpp"
#include <gtest/gtest.h>

#define TEST_SCRIPT(src) #src

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

TEST_F(TestVM, prints_correctly)
{
  const char* script = {
#include "scripts/print_script.ss"
  };

  this->vm->run_script(script);

  ASSERT_EQ(this->ostream->str(), "true\nhello world\n");
}

TEST_F(TestVM, setting_and_getting_vars)
{
  const char* script = {
#include "scripts/var_script.ss"
  };

  this->vm->set_var("value", Value("test"));
  this->vm->run_script(script);

  EXPECT_EQ(this->ostream->str(), "test\n");
  EXPECT_EQ(this->vm->get_var("value"), Value(true));
}

TEST_F(TestVM, blocks)
{
  const char* script = {
#include "scripts/block_script.ss"
  };

  this->vm->run_script(script);

  EXPECT_EQ(this->ostream->str(), "string\n");
}

TEST_F(TestVM, if_statements)
{
  const char* script = {
#include "scripts/if_script.ss"
  };

  this->vm->run_script(script);

  EXPECT_EQ(this->ostream->str(), "true\nafter\n");
}

TEST_F(TestVM, if_else_statements)
{
  const char* script = {
#include "scripts/if_else_script.ss"
  };

  this->vm->run_script(script);

  EXPECT_EQ(this->ostream->str(), "true\nfalse\ny\n");
}
