
#ifndef BINARYTREE
#define BINARYTREE 

#include <iostream>
#include <stack>
#include <queue>
#include <unistd.h>
using namespace std;


template<class T>
struct binTreeNode{
    T info;
    binTreeNode<T>* lchild,*rchild;
    //ctors
    //空节点
    binTreeNode(){
      lchild = rchild = nullptr;
    }
    //叶子节点
    binTreeNode(const T& Info) : info(Info),lchild(nullptr),rchild(nullptr){}
    //内部节点
    binTreeNode(const T& Info,binTreeNode* left,binTreeNode* right):info(Info),lchild(left),rchild(right){}

    //判断是否为叶子节点
    bool isLeaf() const {return (lchild == nullptr) & (rchild == nullptr);}
};


//递归遍历

template<class T>
void visit(binTreeNode<T>* p){
  cout << p->info << ' ';
}

//前序遍历
template<class T>
void preorder(binTreeNode<T>* root){
  //进入右子树时，其可能为空，也要返回到上一层
  if(root == nullptr)
    return;
  //边界,遇到叶子节点就返回到上一层       ---> 这一边界也可以不要,从叶子节点进入下一层 --> nullptr 又返回到改层
  if(root->isLeaf()){
   visit(root);
    return;
  }
  //递归规则
  visit(root);
  preorder(root->lchild);
  preorder(root->rchild);
}

//前序遍历
template<class T>
void preorder2(binTreeNode<T>* root){
  //进入右子树时，其可能为空，也要返回到上一层
  if(root == nullptr)
    return;
  //边界,遇到叶子节点就返回到上一层       ---> 这一边界也可以不要,从叶子节点进入下一层 --> nullptr 又返回到改层
  if(root->isLeaf()){
    delete root;
    return;
  }
  //递归规则
  delete root;
  preorder2(root->lchild);
  preorder2(root->rchild);
}

template<class T>
void noRePreorder(binTreeNode<T>* root){
  stack<binTreeNode<T>*> s;
  s.push(root);
  binTreeNode<T>* tmp=root;
  do{
      //先访问，再深入左子树
    while(tmp != nullptr){
      //深入到叶子节点，不压入，下次就该回溯
      if(!tmp->isLeaf())
        s.push(tmp);
      visit(tmp);
      tmp = tmp->lchild;
    }
     //取出栈顶元素进行回溯并进入右子树
    tmp = s.top()->rchild;
    s.pop();
  }while(!s.empty() || tmp != nullptr); //回溯到root时，栈为空，但仍然可以进入右子树
}

//中序遍历
template<class T>
void inOrder(binTreeNode<T>* root){
  //边界理由同上
  if(root == nullptr)
    return;
  if(root->isLeaf()){
    visit(root);
    return;
  }
  //不断递归深入左子树
  inOrder(root->lchild);
  //返回到上一层:进行访问
  visit(root);
  //进入右子树递归
  inOrder(root->rchild);
}


template<class T>
void noReInOrder(binTreeNode<T>* root){
  stack<binTreeNode<T>*> s;
  binTreeNode<T>* tmp = root;
  do{
    //不断深入左子树，压入回溯点
    while(tmp != nullptr){
      s.push(tmp);
      tmp = tmp->lchild;
    }
    //回溯，访问，进入右子树
    tmp = s.top();
    s.pop();
    visit(tmp);
    tmp = tmp->rchild;
  }while(!s.empty() || tmp != nullptr); //同上
}

//后续遍历
template<class T>
void postOrder(binTreeNode<T>* root){
  if(root == nullptr)
    return;

  postOrder(root->lchild);
  postOrder(root->rchild);
  //从右子树回来后再访问
  visit(root);
}


template<class T>
void noRePostOrder(binTreeNode<T>* root){
  stack<binTreeNode<T>*> s;
  binTreeNode<T>* tmp =root;
  binTreeNode<T>* lastVisited = nullptr;
  do{
    if(tmp != nullptr){
      //压入回溯点，进入左子树
      s.push(tmp);
      tmp = tmp->lchild;
    }else{ //进行回溯
      tmp = s.top();
      //若右子树存在且不是最近被访问的,则进入右子树-->第一种回溯,并以此为根
      if(tmp->rchild != nullptr &&  tmp->rchild != lastVisited)
        tmp = tmp->rchild;
      else{ //要么右子树不存在，要么已经访问过,访问回溯点
        s.pop();
        visit(tmp);
        lastVisited = tmp;  //记录上一次访问点
        tmp = nullptr;   //选择下一个回溯点的关键
      }
    }
  }while(tmp != nullptr || !s.empty());
    //要保证最后的根节点被访问到
}

//层次遍历---> 采用队列进行广度遍历
template<class T>
void levelOrder(binTreeNode<T>* root){
    queue<binTreeNode<T>*> q;
    binTreeNode<T>* tmp = root;
    while(true){  
      visit(tmp);
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

//找到最多节点的一层
template<class T>
void maxNumOfLevel(binTreeNode<T>* root){
  int currN = 1;     //本层的节点个数
  int nextN = 0;     //下一层的节点个数
  int maxNum =0,maxLevel =0,currLevel=0;;
  queue<binTreeNode<T>*> q;
  q.push(root);
  binTreeNode<T>* tmp = nullptr;
  while(true){
    //从队列中取出本层个数的节点
    for(int i=0;i<currN;i++){
      tmp = q.front();
      q.pop();
      //得到下层节点，计数并加入到队列中
      if(tmp->lchild){
        q.push(tmp->lchild);
        ++nextN;
      }
      if(tmp->rchild){
        q.push(tmp->rchild);
        ++nextN;
      }
    }
    //数值比较
    if(currN > maxNum){
      maxNum = currN;
      maxLevel = currLevel;
    }
    if(q.empty())
      break;
    //进入下一层
    currN = nextN;
    nextN = 0;
    ++currLevel;
  }
  cout << "Line: " << maxLevel << " has the maximum number" << "(" << maxNum << ")" << "nodes\n";
}
//完全括号化的中缀表达式:每个输出元素都加上()
template<class T>
void infix(binTreeNode<T>* root){
  if(root != nullptr){
     cout << '(';
     infix(root->lchild);
     cout << root->info;
     infix(root->rchild);
     cout <<')';
  }
}


//二叉树高度
template<class T>
int getHeight(binTreeNode<T>* root){
  if(root->isLeaf())
    return 0;
  return max(getHeight(root->lchild),getHeight(root->rchild))+1;   //左子树高度和右子树高度进行比较，选择较大的再加1
}




template<class T>
int findRootIdx(T in[],T x){
  for(int i=0;;i++)
    if(in[i] == x)
      return i;
}

struct range{
  int begin,end;
  
};

template<class T>
binTreeNode<T>* binTreeFromOrdering(T pre[],range r1,T in[],range r2){
    if(r2.begin == r2.end)
        //递归到叶子节点
      return new binTreeNode<T>(in[r2.begin]);
    //左子树或者右子树不存在
    if(r2.end < r2.begin)
      return nullptr;
    T root = pre[r1.begin];
    binTreeNode<T>* rootNode = new binTreeNode<T>(root);
    //根节点在in中的下标
    int rIdx = findRootIdx(in,root);
    cout << rIdx<< endl;
    sleep(1);
    //左子树大小
    int lsize = rIdx - r2.begin;
    //递归进入左子树
    rootNode->lchild = binTreeFromOrdering(pre,{r1.begin+1,r1.begin+lsize},in,{r2.begin,rIdx-1});
    //进入右子树1
    rootNode->rchild = binTreeFromOrdering(pre,{r1.begin+lsize+1,r1.end},in,{rIdx+1,r2.end});
    return rootNode;
}

template<class T>
binTreeNode<T>* binTreeFromOrdering2(T pre[],range r1,T post[],range r2){
   //叶子节点
   if(r2.begin == r2.end)
     return new binTreeNode<T>(post[r2.begin]);
   //左子树或者右子树不存在
   if(r2.begin > r2.end)
     return nullptr;
   T root = pre[r1.begin];
   binTreeNode<T>* rootNode = new binTreeNode<T>(root);
   //左子树根节点在post中的下标
   T leftRoot = pre[r1.begin+1];
   int leftIdx = findRootIdx(post,leftRoot);
   //左子树大小
   int lsize = leftIdx - r2.begin+1;
   rootNode->lchild = binTreeFromOrdering2(pre,{r1.begin+1,r1.begin+lsize},post,{r2.begin,leftIdx});
   rootNode->rchild = binTreeFromOrdering2(pre,{r1.begin+lsize+1,r1.end},post,{leftIdx+1,r2.end-1});
   return rootNode;
}

#endif 
