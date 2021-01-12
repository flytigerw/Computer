



#ifndef BINARYSEARCHTREE
#define BINARYSEARCHTREE 

#include "bsTree.h"
#include "../BinaryTree/linkedBinTree.h"

template<class K,class V>
class binarySearchTree : public bsTree<K,V>,public linkedBinTree<pair<K,V>>{
  
  public:
    using LBT = linkedBinTree<pair<K,V>>;

    bool empty() const override {return LBT::treeSize == 0;}
    int size() const override {return LBT::treeSize;}
    void ascend() const override {LBT::inOrderOutput();}
    pair<K,V>* find(const K& theKey) const override;
    void insert(const pair<K,V>& ) override;
    void erase(const K& theKey) override;

    void outputInRange(K low,K high);
  private:

};

template<class K,class V>
pair<K,V>* binarySearchTree<K,V>::find(const K& theKey) const {
        binTreeNode<pair<K,V>>* p = LBT::root;
        while(p != nullptr){
          if(p->info.first == theKey)
            return p;
          if(p->info.first > theKey)
            //偏小进入左子树
            p = p->lchild;
            //偏大进入右子树
          else 
            p = p->rchild;
        }
        return nullptr;
}

template<class K,class V>
void binarySearchTree<K,V>::insert(const pair<K,V>& thePair){
    binTreeNode<pair<K,V>>* p =LBT::root,*pp = nullptr;
    while(p){
      if(p->info.first == thePair.first){
        p->info.second = thePair.second;
        return;
      }
      //偏下进入左子树
      pp = p;
      if(p->info.first > thePair.first)
          p = p->lchild;
      //偏大进入右子树
      else 
          p = p->rchild;
    }
    //查找失败. pp指向查找中断节点 
    p = new binTreeNode<pair<K,V>>(thePair);
    //有可能root为nullptr
    if(pp == nullptr){
      LBT::root = p;
      return;
    }
    //pp为新节点的父节点
    if(pp->info.first < thePair.first)   //偏大为右节点
      pp->rchild = p;
    else 
      pp->lchild = p;
    LBT::treeSize++;
}

template<class K,class V>
void binarySearchTree<K,V>::erase(const K& theKey){
  binTreeNode<pair<K,V>>* p = LBT::root,*pp = nullptr;
  //先查找
  while(p){
    if(p->info.first == theKey)
      break;
    pp = p;
    if(p->info.first > theKey)
      p = p->lchild;
    else 
      p = p->rchild;
  }
  //是否找到的检验
  if(p == nullptr) //没找到
    return;
  //有两颗子树情况
  if(p->lchild && p->rchild){
    //找左子树的最右节点
    binTreeNode<pair<K,V>>* s = p->lchild,*sp = p;
    while(s->rchild){
      sp = s;
      s = s->rchild;
    }
    
    //p->info = s->info is not permitted since it's constant
    binTreeNode<pair<K,V>> q = new binTreeNode<pair<K,V>>(s->info.first,pp->lchild,pp->rchild);
     
    //删除节点后，用新节点进行替代
    //删除的是根节点 ---> pp == nullptr      ---> 需要改变root的指向
    if(pp == nullptr)
      LBT::root = q;
    if(p == pp->rchild)
      pp->rchild = q;
    else if(p == pp->lchild)
      pp->lchild = q;
    
    //改变pp和p的指向，让其变为只有一颗子树的情况
    if(sp == p)  //p的左子树就一个节点s
      pp = q;
    else 
      pp = sp;
    //删除节点
    delete p;
    p = s;
  }

  //最多只有一个孩子
  //先把子树暂存
  binTreeNode<pair<K,V>>* tmp;
  if(p->lchild)
    tmp=p->lchild;
  else 
    tmp=p->rchild;             //有可能p为leaf ---> tmp依旧为nullptr
  
  if(p == LBT::root)          //删除根节点
    LBT::root = tmp;
  else{              //含有父节点
    if(p == pp->lchild)
      pp->lchild = tmp;
    else 
      pp->rchild = tmp;
  }
  
  LBT::treeSize--;
  delete p;
}

//overload << for pair 
template<class K,class V>
ostream& operator<<(ostream& out,const pair<K,V>& p){
  out << p.first << '-' << p.second;
  return out;
}
#endif
