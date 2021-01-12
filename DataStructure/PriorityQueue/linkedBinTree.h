#ifndef LINKEDBINTREE
#define LINKEDBINTREE 

#include <iostream>
#include <queue>
#include "binTreeNode.h"
#include "binTreeBase.h"
#include "nodeInfo.h"
using namespace std;

template<class T>
class linkedBinTree : public binTree<binTreeNode<T>>{
  public:
    using Do = void(*) (binTreeNode<T>*);
    void makeTree(const T&,linkedBinTree<T>&,linkedBinTree<T>&);
    //remove and return the left tree
    linkedBinTree<T>& removeLeftSubtree();
    linkedBinTree<T>& removeRightSubtree();
    //ctor
    linkedBinTree():root(nullptr),treeSize(0){}
    //dctor
    ~linkedBinTree(){erase();}
    //override functions
    bool empty() const{return treeSize == 0;}
    int size() const {return treeSize;}
    void preOrder(Do doIt){doWhat = doIt;postOrder(root);}
    void inOrder(Do doIt) {doWhat = doIt;inOrder(root);}
    void postOrder(Do doIt) {doWhat = doIt;postOrder(root);}
    void levelOrder(Do doIt);
    
    void preOrderOutput(){postOrder(output);cout << endl;}
    void inOrderOutput(){inOrder(output);cout << endl;}
    void postOrderOutput(){postOrder(output);cout << endl;}
    //辅助函数
    void erase(){
      postOrder(dispose);
      root = nullptr; 
      treeSize = 0;
    }
    
    //特有函数
    T* getRootInfo() const;
    int getHeight() const {return height(root);}

  protected:
    binTreeNode<T>* root;
    int treeSize;
    static int count;
    static void preOrder(binTreeNode<T>*);
    static void inOrder(binTreeNode<T>*);
    static void postOrder(binTreeNode<T>*);

    static int height(binTreeNode<T>*);
    static int countNodes(binTreeNode<T>* t){
      doWhat = addToCount;
      count = 0;
      preOrder(t);
      return count;
    }

    //遍历时具体要干嘛
    static void(*doWhat)(binTreeNode<T>*);
    static void dispose(binTreeNode<T>* t){delete t;}
    static void output(binTreeNode<T>* t){
      cout << t->info << ' ';
    }
    static void addToCount(binTreeNode<T>*){
      count++;
    }
    static void placeBooster(binTreeNode<T>*);
};

template<class T>
void(* linkedBinTree<T>::doWhat)(binTreeNode<T>*);

template<class T>
void linkedBinTree<T>::placeBooster(binTreeNode<T>* node){
      //初始化
      node->info.degradeToLeaf = 0;
      //计算左子树的衰减量
      auto tmp = node->lchild;
      if(tmp != nullptr){
        int degradation = tmp->info.degradeToLeaf+tmp->info.degradeFromParent;
        if(degradation > tolerance){
          //在此处设置放大器
          node->info.boosterHere = true;
          //更新node的衰减量
          node->info.degradeToLeaf= tmp->info.degradeFromParent;
        }else 
          node->info.degradeToLeaf = degradation;
      }
      //再考察右子树
      tmp = node->rchild;
      if(tmp != nullptr){
        int degradation = tmp->info.degradeToLeaf+tmp->info.degradeFromParent;
        if(degradation > tolerance){
          node->info.boosterHere = true;
          node->info.degradeToLeaf= tmp->info.degradeFromParent;
        }else 
          node->info.degradeToLeaf = degradation;
      }
};


template<class T>
T* linkedBinTree<T>::getRootInfo() const{
  return (treeSize==0) ? nullptr : &root->root;
}

template<class T>
void linkedBinTree<T>::makeTree(const T& Info,linkedBinTree<T>& left,linkedBinTree<T>& right){
  root = new binTreeNode<T>(Info,left.root,right.root);
  treeSize = left.treeSize+right.treeSize+1;
  //Deny access from trees left and right 
  left.root = right.root = nullptr;
  left.treeSize=right.treeSize=0;
}

template<class T>
linkedBinTree<T>& linkedBinTree<T>::removeLeftSubtree(){
      if(treeSize == 0) 
        return linkedBinTree<T>();

      linkedBinTree<T> leftSubtree;
      leftSubtree.root = root->lchild;
      root->lchild = nullptr;
      count = 0;
      leftSubtree.treeSize = countNodes(leftSubtree.root);
      treeSize -= leftSubtree.treeSize;
      return leftSubtree;
}

template<class T>
linkedBinTree<T>& linkedBinTree<T>::removeRightSubtree(){
      if(treeSize == 0) 
        return linkedBinTree<T>();

      linkedBinTree<T> rightSubtree;
      rightSubtree.root = root->rchild;
      root->rchild = nullptr;
      count = 0;
      rightSubtree.treeSize = countNodes(rightSubtree.root);
      treeSize -= rightSubtree.treeSize;
      return rightSubtree;
}

template<class T>
void linkedBinTree<T>::preOrder(binTreeNode<T>* t){
  if(t != nullptr){
    linkedBinTree<T>::doWhat(t);
    preOrder(t->lchild);
    preOrder(t->rchild);
  }
}

template<class T>
void linkedBinTree<T>::inOrder(binTreeNode<T>* t){
  if(t != nullptr){
    inOrder(t->lchild);
    linkedBinTree<T>::doWhat(t);
    inOrder(t->rchild);
  }
}

template<class T>
void linkedBinTree<T>::postOrder(binTreeNode<T>* t){
  if(t != nullptr){
    postOrder(t->lchild);
    postOrder(t->rchild);
    linkedBinTree<T>::doWhat(t);
  }
}


template<class T>
void linkedBinTree<T>::levelOrder(void(*doIt)(binTreeNode<T>*) ){
   queue<binTreeNode<T>*> q;
   binTreeNode<T>* tmp = root;
   while(true){
     doIt(tmp);
     if(tmp->lchild)
       q.push(tmp->lchild);
     if(tmp->rchild)
       q.push(tmp->rchild);
     if(q.empty())
       break;
     tmp = q.front();
     q.pop();
   }
}


template<class T>
int linkedBinTree<T>::height(binTreeNode<T>* t){
  if(!t)
    return 0;
  return max(height(t->lchild),height(t->rchild))+1;
}
#endif
