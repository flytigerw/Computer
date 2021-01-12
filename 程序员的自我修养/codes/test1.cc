
#include <iostream>

namespace myname{
  int var = 42;
}

extern "C" int _ZN6myname3varE;

int main(){
  std::cout << _ZN6myname3varE << std::endl;
  return 0;
}
