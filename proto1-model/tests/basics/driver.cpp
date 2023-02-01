#include <sstream>
#include <stdexcept>

#include <proto1-model/version.hpp>
#include <proto1-model/proto1-model.hpp>

#undef NDEBUG
#include <cassert>

int main ()
{
  using namespace std;
  using namespace proto1_model;

  // Basics.
  //
  {
    ostringstream o;
    say_hello (o, "World");
    assert (o.str () == "Hello, World!\n");
  }

  // Empty name.
  //
  try
  {
    ostringstream o;
    say_hello (o, "");
    assert (false);
  }
  catch (const invalid_argument& e)
  {
    assert (e.what () == string ("empty name"));
  }
}
