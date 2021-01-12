

#include <iostream>
#include "myExceptions.h"
using namespace std;


template<class T>
struct listNode{
   T info;
   listNode<T>* next;
   listNode(int x):info(x){next = nullptr;}
};

template<class T>
int findKthToTail(listNode<T>* head,int k){
  if(head  == nullptr || k==0) 
    throw illegalParameterValue();

  listNode<T>* p2 ,*p1;
  p2 = p1 = head;
  //第二根指针先跑，二者保持k个身位差
  for(int i=0;i<k;i++){
    if(p1 == nullptr)
      throw illegalParameterValue();
    p1 = p1->next;
  }
  while(p1 != nullptr){
    p2 = p2->next;
    p1 = p1->next;
  }
  return p2->info;
}

int main(){
  listNode<int> n1(1);
  listNode<int> n2(2);
  listNode<int> n3(3);
  listNode<int> n4(4);
  listNode<int> n5(5);
  listNode<int> n6(6);
  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;
  n4.next = &n5;
  n5.next = &n6;
  cout << findKthToTail(&n1,2) << endl;
}
