#pragma once

#include <iostream>
#include <memory>

namespace ss
{
  constexpr bool DISASSEMBLE_CHUNK        = false;
  constexpr bool DISASSEMBLE_INSTRUCTIONS = false;
  constexpr bool PRINT_STACK              = false;
  constexpr bool PRINT_CONSTANTS          = false;
  constexpr bool ECHO_INPUT               = false;

  template <typename T>
  concept Writable = requires(T& t)
  {
    std::cout << t;
  };

  template <typename T>
  concept Readable = requires(T& t)
  {
    std::cin >> t;
  };

  template <typename T>
  concept LineReadable = requires(T& t)
  {
    std::getline(std::cin, t, '\n');
  };

  class VMConfig
  {
   public:
    static VMConfig basic;

    VMConfig(std::istream* istream = &std::cin, std::ostream* ostream = &std::cout);
    ~VMConfig() = default;

    template <Writable... Args>
    void write(Args&&... args)
    {
      if (this->ostream != nullptr) {
        (((*this->ostream) << std::forward<Args>(args)), ...);
      }
    }

    template <Writable... Args>
    void write_line(Args&&... args)
    {
      write(args..., '\n');
    }

    template <Readable T>
    void read(T& t)
    {
      if (this->istream != nullptr) {
        (*istream) >> t;
      }
    }

    template <LineReadable T>
    void read_line(T& t, char delim = '\n')
    {
      if (this->istream != nullptr) {
        std::getline(*this->istream, t, delim);
      }
    }

    void reset_istream();
    void reset_ostream();

   private:
    std::istream* istream;
    std::ostream* ostream;

    std::shared_ptr<std::ios> istream_initial_state;
    std::shared_ptr<std::ios> ostream_initial_state;
  };
}  // namespace ss
