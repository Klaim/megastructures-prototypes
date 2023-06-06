#include <proto2-view.hpp>

#include <ostream>
#include <stdexcept>

using namespace std;

namespace proto2_view
{
  void say_hello (ostream& o, const string& n)
  {
    if (n.empty ())
      throw invalid_argument ("empty name");

    o << "Hello, " << n << '!' << endl;
  }
}
