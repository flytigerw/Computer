


#include <stack>
#include <iterator>

using namespace std;
template<class T>
struct node{
   T info; 
   node<T>* lchild,rchild;
   //三种构造函数
   bool isLeaf() const;
};


template<class T>
void preorder(node<T>* root){
    
  if(root->isLeaf()){
    visit(root);
    return;
  }
  visit(root);
  preorder(root->lchild);
  preorder(root->rchild);
}

template<class T>
void noRePreorder(node<T>* root){
  stack<node<T>*> s;
  node<T>* tmp = root; 
  
  do{

    while(tmp != nullptr){
      if(!tmp->isLeaf())
        s.push(tmp);
      visit(tmp);
      tmp = tmp->lchild;
    }

    tmp = s.top()->rchild;
    s.pop();
  }while(!s.empty() && tmp != nullptr);
}


template<class T>
void per(T list[],int k,int m){
  if(k == m){
    copy(list,list+m+1,os)
  }
}





