

#ifndef BINTREENODE
#define BINTREENODE 

template<class T>
struct binTreeNode{
  T info;
  binTreeNode<T>*rchild,*lchild;
  //ctors 
  //空节点
  binTreeNode() : rchild(nullptr),lchild(nullptr){}
  //叶子节点
  binTreeNode(const T& Info):info(Info),rchild(nullptr),lchild(nullptr){}
  //内部节点
  binTreeNode(const T& Info,binTreeNode<T>* left,binTreeNode<T>* right):info(Info),lchild(left),rchild(right){}

  bool isLeaf() const {return rchild == nullptr && lchild == nullptr;}
};

#endif
