#pragma once

#include <iosfwd>
#include <string>

#include <proto1-model/export.hpp>

namespace proto1_model
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  PROTO1_MODEL_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
