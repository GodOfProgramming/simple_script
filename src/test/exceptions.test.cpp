#include "helpers.hpp"
#include "ss/exceptions.hpp"
#include <gtest/gtest.h>

using ss::CompiletimeError;
using ss::RuntimeError;

TEST(CompiletimeError, METHOD(what, returns_a_message))
{
  CompiletimeError err("a message");
  EXPECT_STREQ(err.what(), "a message");
}

TEST(RuntimeError, METHOD(what, returns_a_message))
{
  RuntimeError err("a message");
  EXPECT_STREQ(err.what(), "a message");
}
