

#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <vector>
#include <unistd.h>
using namespace std; 

struct parentBinNode{
   int info;
   parentBinNode* rchild,*lchild,*parent;
   
   parentBinNode(int i = 0) : info(i){rchild = lchild = parent = nullptr;}

   bool isLeaf() const{return rchild == nullptr && lchild == nullptr;}
};

struct binNode{
  int info;
  binNode* lchild,*rchild;
  binNode(int i = 0) : info(i) {lchild = rchild = nullptr;}
   bool isLeaf() const{return rchild == nullptr && lchild == nullptr;}
};


queue<char> q;
void serialize(binNode* root){
  if(root == nullptr){
    q.push('$');
    return;
  }
  q.push((char)root->info);
  serialize(root->lchild);
  serialize(root->rchild);
}

//若读取到$,返回false
//否则返回true，并存储已读取的字符
bool readChar(int& result){
  if(q.empty())
    return false;
  char c;
  c = q.front();
  q.pop();
  if(c == '$')
    return false;
  else{
    result = (int)c;
    return true;
  }
}

void deserialize(binNode*& root,fstream& in){
  int num;
  if(readChar(num)){
    root = new binNode(num);
    deserialize(root->lchild,in);
    deserialize(root->rchild,in);
  }
}

void printInLevels(binNode* root){
  if(root == nullptr)
    return;

  queue<binNode*> q;
  q.push(root);
  binNode* tmp;
  int lastCount=1;
  int count = 0;
  while(!q.empty()){
    for(int i=1;i<= lastCount; i++){
        tmp = q.front();
        q.pop();
        cout << tmp->info << ' ';
        if(tmp->lchild){
          q.push(tmp->lchild);
          ++count;
        }
        if(tmp->rchild){
          q.push(tmp->rchild);
          ++count;
        }
    }
    cout << endl;
    lastCount = count;
    count  = 0;
  }

}

static vector<int> routes;
void printRoutes(){
  for(const auto& x: routes)
    cout << x << ' ';
  cout << endl;
}
void search(binNode* now,int k,int last){
  if(now == nullptr){
    return;
  }
  if(last+now->info > k){
    return;
  }
  routes.push_back(now->info);
  if(last+now->info == k){
    printRoutes();
    return;
  }
  if(now->lchild){
    search(now->lchild,k,last+now->info);
    routes.pop_back();
  }
  if(now->rchild){
    search(now->rchild,k,last+now->info);
    routes.pop_back();
  }
}

void printLinkList(binNode* root){
  binNode* tmp = root;
  while(tmp){
    cout << tmp->info << endl;
    tmp = tmp->rchild;
  }
}
void convertSearchTreeToList(binNode* root){
  if(root == nullptr || root->isLeaf())
    return;
  //中序遍历
  stack<binNode*> s;
  binNode* tmp = root,*last = nullptr,*first;
  do{
    //不断压入左子树根节点
    while(tmp != nullptr){
      s.push(tmp);
      tmp = tmp->lchild;
    }
    //回溯
    tmp = s.top();
    s.pop();
    if(first==nullptr)
      first = tmp;
    if(last != nullptr){
      //判断中序遍历时，该节点与上一节点的关系
      if(tmp->lchild == last){
        last->rchild = tmp;
      }
      else if(tmp->rchild == last)
        last->lchild = tmp;
      else {
        last->rchild = tmp;
        tmp ->lchild = last;
      }
    }
    last = tmp;
    tmp = tmp->rchild;
  }while(!s.empty() || tmp != nullptr);
  printLinkList(first);
}
void searchRoutes(binNode* root,int k){
  if(root == nullptr)
    return;
  search(root,k,0);
}

bool isSubtree(binNode* A,binNode* R){
      queue<binNode*> qA;
      queue<binNode*> qR;
      qA.push(A);
      qR.push(R);
      binNode* tmpA;
      binNode* tmpR;
      while(!qA.empty()){
          tmpA = qA.front();
          qA.pop();
          tmpR = qR.front();
          qR.pop();
          if(tmpA->info != tmpR->info)
            return false;
          if(tmpA->lchild && tmpR->lchild){
            qA.push(tmpA->lchild);
            qR.push(tmpR->rchild);
          }else if(!tmpA->lchild && !tmpR->lchild){

          }else return false;

          if(tmpA->rchild && tmpR->rchild){
            qA.push(tmpA->rchild);
            qR.push(tmpR->rchild);
          }else if(!tmpA->rchild && !tmpR->rchild){

          }else return false;
      }
      return false;
}
int error ;
bool isTheSubtree(binNode* A,binNode* B){
    if(A == nullptr || B == nullptr)
      return false;
    queue<binNode*> q;
    q.push(B);
    binNode* tmp;
    while(!q.empty()){
        tmp = q.front();
        q.pop();
        if(tmp->info == A->info)
          if(isSubtree(A,tmp))
            return true;
        if(tmp->lchild)
          q.push(tmp->lchild);
        if(tmp->rchild)
          q.push(tmp->rchild);
    }
    return false;
}


void findNextOfInOrder(parentBinNode* root,parentBinNode* node, parentBinNode*& next){
        if(root == nullptr || root->isLeaf() || node == nullptr)
          return;
        if(node->isLeaf()){
          if(node == node->parent->lchild){
            next = node->parent;
          }
          else{
            auto parent = node->parent;
            while(parent != parent->parent->lchild)
              parent = parent->parent;
            next = parent->parent;
          }
          return;
        }

        if(node->rchild != nullptr){
          auto tmp = node->rchild;
          while(tmp->lchild != nullptr)
            tmp = tmp->lchild;
          next = tmp;
        }
}

int main(){
  binNode n1(10);
  binNode n2(6);
  binNode n3(14);
  binNode n4(4);
  binNode n5(8);
  binNode n6(12);
  binNode n7(16);
  n1.lchild = &n2;
  n1.rchild = &n3;
  n2.lchild = &n4;
  n2.rchild = &n5;
  n3.lchild = &n6;
  n3.rchild = &n7;
  test(&n1);
}
