#include "util.hpp"

#include <fstream>

namespace ss
{
  namespace util
  {
    auto load_file_to_string(std::string filename) -> std::string
    {
      std::string                            contents;
      std::ifstream                          istr(filename);
      std::istreambuf_iterator<char>         input_iter(istr), empty_iter;
      std::back_insert_iterator<std::string> string_inserter(contents);
      std::copy(input_iter, empty_iter, string_inserter);
      return contents;
    }
  }  // namespace util
}  // namespace ss
