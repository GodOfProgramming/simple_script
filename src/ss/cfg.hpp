#pragma once

#include <iostream>
#include <memory>

namespace ss
{
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
    std::getline(std::cin, t);
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
    void read_line(T& t)
    {
      if (this->istream != nullptr) {
        (*istream) >> t;
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
