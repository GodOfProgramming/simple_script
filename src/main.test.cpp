#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[])
{
  // This can be an ofstream as well or any other ostream
  std::stringstream buffer;
  // Redirect cout to our stringstream buffer or any other ostream
  std::cout.rdbuf(buffer.rdbuf());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
