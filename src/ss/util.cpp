#include "util.hpp"

#include <fstream>

namespace ss
{
  namespace util
  {
    auto stream_to_string(std::istream& stream) -> std::string
    {
      std::string contents;
      std::istreambuf_iterator<char> input_iter(stream), empty_iter;
      std::back_insert_iterator<std::string> string_inserter(contents);
      std::copy(input_iter, empty_iter, string_inserter);
      return contents;
    }
  }  // namespace util
}  // namespace ss
