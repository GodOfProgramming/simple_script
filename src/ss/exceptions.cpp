#include "exceptions.hpp"

namespace ss
{
  CompiletimeError::CompiletimeError(std::string msg): message(msg) {}

  auto CompiletimeError::what() const noexcept -> const char*
  {
    return this->message.c_str();
  }

  RuntimeError::RuntimeError(std::string msg): message(msg) {}

  auto RuntimeError::what() const noexcept -> const char*
  {
    return this->message.c_str();
  }
}  // namespace ss
