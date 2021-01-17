#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

namespace
{
  constexpr bool REROUTE_STDOUT = false;
}

int main(int argc, char* argv[])
{
  // std::ofstream null("/dev/null");

  // std::streambuf* old = std::cout.rdbuf();

  // std::cout.rdbuf(null.rdbuf());

  testing::InitGoogleTest(&argc, argv);

  int res = RUN_ALL_TESTS();

  // std::cout.rdbuf(old);

  return res;
}
