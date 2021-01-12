

#include <iostream>
#include <stack>
using namespace  std;

template<class T>
struct node{
  T data;
  node* lchild,*rchild;
};

//height为子树的高度，通过引用传给上层
template<class T>
void postoder(node<T>* root,int& height,bool& res){
  if(root == nullptr || !res){
    height = 0;
    return;
  }
  int lchild_height =0,rchild_height= 0;
  postoder(root->lchild,height,res);
  lchild_height += height;
  postoder(root->rchild,res);
  rchild_height += height;
  height = std::max(lchild_height,rchild_height)+1;
  if(abs(lchild_height - rchild_height) > 1){
    res = false;
    return;
  }
}
int main(){

  return 0;
}
