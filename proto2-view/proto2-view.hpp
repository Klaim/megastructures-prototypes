#pragma once

#include <iosfwd>
#include <string>

#include <export.hpp>

namespace proto2_view
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  PROTO2_VIEW_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
