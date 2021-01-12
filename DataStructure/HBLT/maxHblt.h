#ifndef MAXHBLT
#define MAXHBLT 

#include "linkedBinTree.h"
#include "maxPriorityQueue.h"
#include "myExceptions.h"
#include "binTreeNode.h"
#include <sstream>
#include <queue>
using namespace std;

template <class T>
class maxHblt :public maxPriorityQueue<T>, public linkedBinTree<pair<int,T>> {
  //可以作为最大优先级队列 ---> 二叉树指针版而非数组版
  //pair<int,T> --> 第一个为s(x),第二个为元素
  public:
    using hbltNode = binTreeNode<pair<int,T>>;
    using space = linkedBinTree<pair<int,T>>;
    //override maxPriorityQueue functions
    bool empty() const override{return space::treeSize == 0;}
    int size() const override{return space::treeSize;}
    const T& top()override {
      if(space::treeSize == 0)
        throw queueEmpty();
      return space::root->info.second;
    }
    void pop() override;
    void push(const T&)override ;
    //other funcs 
    void initialize(T*,int);
    void meld(maxHblt<T>& hblt){
      //meld *this and hblt 
      meld(space::root,hblt.space::root);
      space::treeSize += hblt.space::treeSize;
      hblt.space::root = nullptr;
      hblt.space::treeSize = 0;
    }
    void output(){
      space::postOrder(hbltOutput);cout << endl;
    }
  private:
   //合并两个hbltTree,顺着节点考察并合并
   void meld(hbltNode*&,hbltNode*&);
   static void hbltOutput(hbltNode* t){
     cout << t->info.second << ' ';
   }
};

template<class T>
void maxHblt<T>::meld(hbltNode*& x,hbltNode*& y){
  //合并以*x,*y的左高树
  //合并后的左高树以x为根,通过引用返回
  if(y == nullptr)
    return;
  if(x == nullptr){  //此时x为上一层X的右子树.将y迁移为X的右子树
    x = y;return;
  }
  
  //neither is empty,swap x and y if necessary 
  if(x->info.second < y->info.second)
    swap(x,y);
  //顺着x的右子树，向下递归
  meld(x->rchild,y);
  if(x->lchild == nullptr){
    //左子树为空，将右子树拉过来作为左子树
    x->lchild = x->rchild;
    x->rchild = nullptr;
    x->info.first = 1;
  }else{
    //比较左右子树的s(x).必要时交换左右子树
    if(x->lchild->info.first < x->rchild->info.first)
      swap(x->lchild,x->rchild);
    //update s value of x
    x->info.first = x->rchild->info.first+1;
  }
}


template<class T>
void maxHblt<T>::push(const T& element){
  //增加一个新节点 等效于 两个hblt结合
  hbltNode* newNode = new hbltNode({1,element});
  
  meld(space::root,newNode);
  space::treeSize++;
}
template<class T>
void maxHblt<T>::pop(){
  //去除根节点 ---> 剩下两颗子树再合并
  if(space::root == nullptr)
    throw queueEmpty();

  hbltNode* left = space::root->lchild,*right=space::root->rchild;
  delete space::root;
  space::root = left;
  meld(space::root,right);
  space::treeSize--;
}


template<class T>
void maxHblt<T>::initialize(T* elements,int size){
  //将所有的elements生成hbltNode并纳入FIFO队列中
  queue<hbltNode*> q;
  //delete old queue 
  space::erase();
  for(int i=0;i<size;i++)
    q.push(new hbltNode({1,elements[i]}));

  //每次两个hbltTree进行合并，知道只有一个hbltTree为止。总共需要size-1次 --> 类似冒泡
  for(int i=0;i<size-1;i++){
    hbltNode* b = q.front();
    q.pop();
    hbltNode* c = q.front();
    q.pop();
    meld(b,c);
    q.push(b);
  }
  if(size>0)
    space::root = q.front();
  space::treeSize = size;
}
#endif
