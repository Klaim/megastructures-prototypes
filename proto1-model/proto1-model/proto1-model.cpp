#include <proto1-model/proto1-model.hpp>

#include <ostream>
#include <stdexcept>

using namespace std;

namespace proto1_model
{
  void say_hello (ostream& o, const string& n)
  {
    if (n.empty ())
      throw invalid_argument ("empty name");

    o << "Hello, " << n << '!' << endl;
  }
}
