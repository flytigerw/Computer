#include "binaryTree.h"

int main(){

  
  binTreeNode<char> n1('+');
  binTreeNode<char> n2('*');
  binTreeNode<char> n3('/');
  binTreeNode<char> n4('a');
  binTreeNode<char> n5('b');
  binTreeNode<char> n6('c');
  binTreeNode<char> n7('d');

  n1.lchild=&n2;
  n1.rchild=&n3;
  
  n2.lchild=&n4;
  n2.rchild=&n5;

  n3.lchild=&n6;
  n3.rchild=&n7;
  int post[] = {4,8,9,5,2,6,7,3,1};
  int pre[] = {1,2,4,5,8,9,3,6,7};
  auto root =binTreeFromOrdering2(pre,{0,8},post,{0,8});
  postOrder(root);
  preorder2(root);
  return 0;
}
