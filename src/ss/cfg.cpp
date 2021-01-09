#include "cfg.hpp"

namespace ss
{
  VMConfig VMConfig::basic;

  VMConfig::VMConfig(std::istream* is, std::ostream* os)
   : istream(is),
     ostream(os),
     istream_initial_state(std::make_shared<std::ios>(nullptr)),
     ostream_initial_state(std::make_shared<std::ios>(nullptr))
  {
    this->istream_initial_state->copyfmt(*this->istream);
    this->ostream_initial_state->copyfmt(*this->ostream);
  }

  void VMConfig::reset_istream()
  {
    this->istream->copyfmt(*this->istream_initial_state);
  }

  void VMConfig::reset_ostream()
  {
    this->ostream->copyfmt(*this->ostream_initial_state);
  }
}  // namespace ss