#include <iostream>
#include <proto1-model/proto1-model.hpp>

int main (int argc, char* argv[])
{
  using namespace std;

  if (argc < 2)
  {
    cerr << "error: missing name" << endl;
    return 1;
  }

  proto1_model::say_hello(cout, argv[1]);
}
