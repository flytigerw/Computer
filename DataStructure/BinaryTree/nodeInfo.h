#ifndef NODEINFO
#define NODEINFO
#include <iostream>

constexpr int tolerance = 2;
using namespace std;
struct nodeIno{
  int degradeFromParent,degradeToLeaf;
  bool boosterHere;
  void output(ostream& out) const{
    out << boosterHere << ' ' << degradeToLeaf << ' ' << degradeFromParent << ' ';
  }
};
ostream& operator<<(ostream& out,const nodeIno& info){
  info.output(out);
  return out;
}
#endif
