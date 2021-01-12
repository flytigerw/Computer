

#ifndef BSTREE
#define BSTREE 

#include "dictionary.h"
template<class K,class V >
class bsTree : public dictionary< K,V>{
  public:
      virtual void ascend()  = 0;
};
#endif
