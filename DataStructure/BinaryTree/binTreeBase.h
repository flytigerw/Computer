

#ifndef BINTREEBASE
#define BINTREEBASE 

//元素类型为二叉树节点
template<class T>
class binTree{
  //遍历时干嘛?
  public:
  virtual ~binTree(){}
  virtual bool empty() const = 0;
  virtual int size() const = 0;
  virtual void preOrder(void((*)(T*))) = 0;
  virtual void inOrder(void((*)(T*))) = 0;
  virtual void postOrder(void((*)(T*))) = 0;
  virtual void levelOrder(void((*)(T*))) = 0;
};
#endif
