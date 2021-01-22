#include "helpers.hpp"
#include "ss/cfg.hpp"
#include <gtest/gtest.h>
#include <sstream>

using ss::VMConfig;

class TestVMConfig: public testing::Test
{
 public:
  void SetUp() override
  {
    this->oss    = std::make_shared<std::ostringstream>();
    this->iss    = std::make_shared<std::istringstream>();
    this->config = std::make_shared<VMConfig>(this->iss.get(), this->oss.get());
  }

 protected:
  std::shared_ptr<std::ostringstream> oss;
  std::shared_ptr<std::istringstream> iss;
  std::shared_ptr<VMConfig>           config;
};

TEST_F(TestVMConfig, METHOD(write, writes_to_output))
{
  this->config->write("hello world");
  EXPECT_EQ(this->oss->str(), "hello world");
  this->oss->str(std::string());
  this->config->write_line("hello world");
  EXPECT_EQ(this->oss->str(), "hello world\n");
}

TEST_F(TestVMConfig, METHOD(read, reads_from_input))
{
  this->iss->str("123 example");

  int         n;
  std::string word;

  this->config->read(n);
  this->config->read(word);

  EXPECT_EQ(n, 123);
  EXPECT_EQ(word, "example");
}

TEST_F(TestVMConfig, METHOD(read_line, can_read_a_whole_line))
{
  this->iss->str(std::string("a multiword sentence"));

  std::string line;

  this->config->read_line(line);

  EXPECT_EQ(line, "a multiword sentence");
}

TEST_F(TestVMConfig, METHOD(reset_ostream, can_set_ostream_back_to_inital_state))
{
  this->config->write(std::setprecision(3), 1.234567);

  EXPECT_EQ(this->oss->str(), "1.23");

  this->config->write(" ", 7.654321);

  EXPECT_EQ(this->oss->str(), "1.23 7.65");

  this->config->reset_ostream();

  this->config->write(" ", 5.4321);

  EXPECT_EQ(this->oss->str(), "1.23 7.65 5.4321");
}
