#pragma once

#include <string>
#include <exception>

#define THROW_COMPILETIME_ERROR(msg) throw CompiletimeError(msg)
#define THROW_RUNTIME_ERROR(msg)     throw RuntimeError(msg)

namespace ss
{
  class CompiletimeError: public std::exception
  {
   public:
    CompiletimeError(std::string msg);
    virtual ~CompiletimeError() = default;

    auto what() const noexcept -> const char*;

   private:
    std::string message;
  };

  class RuntimeError: public std::exception
  {
   public:
    RuntimeError(std::string msg);
    virtual ~RuntimeError() = default;

    auto what() const noexcept -> const char*;

   private:
    std::string message;
  };
}  // namespace ss