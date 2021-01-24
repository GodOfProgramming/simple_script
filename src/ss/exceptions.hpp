#pragma once

#include <exception>
#include <sstream>
#include <string>

namespace ss
{
  class CompiletimeError: public std::exception
  {
   public:
    CompiletimeError(std::string msg);
    virtual ~CompiletimeError() = default;

    template <typename... Args>
    static void throw_err(Args&&... args) noexcept(false)
    {
      std::stringstream ss;
      (((ss) << std::forward<Args>(args)), ...);
      throw CompiletimeError(ss.str());
    }

    auto what() const noexcept -> const char*;

   private:
    std::string message;
  };

  class RuntimeError: public std::exception
  {
   public:
    RuntimeError(std::string msg);
    virtual ~RuntimeError() = default;

    template <typename... Args>
    static void throw_err(Args&&... args) noexcept(false)
    {
      std::stringstream ss;
      (((ss) << std::forward<Args>(args)), ...);
      throw RuntimeError(ss.str());
    }

    auto what() const noexcept -> const char*;

   private:
    std::string message;
  };
}  // namespace ss