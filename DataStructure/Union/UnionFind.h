

#pragma once 


#include <memory>



struct UnionFindNode{
  int parent;  
  bool root;

  UnionFindNode(){     
    parent = 1,root = true; 
  }
};

std::unique_ptr<int> A;

