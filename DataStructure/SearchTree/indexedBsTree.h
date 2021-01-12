

#ifndef INDEXEDBSTREE
#define INDEXEDBSTREE 

#include "bsTree.h"

template<class K,class V>
class indexedBsTree : public bsTree<K,V>{
    
  public:
    virtual pair<K,V>* getPair(int) const = 0;
    virtual void deletPair(int) = 0;
};

#endif
