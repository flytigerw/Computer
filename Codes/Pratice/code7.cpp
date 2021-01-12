
#include <iostream>
#include <stack>
using namespace std;


struct node{
    int info;
    node* next;
    node(int i) : info(i),next(nullptr){}
};


void printLink(node* head){
  while(head != nullptr){
    cout << head->info << ' ';
    head = head->next;
  }
  cout << endl;
}
void reverseLink(node*& head){
  if(head == nullptr || head->next == nullptr)
    return;
  node* p3= head,*p2=p3->next,*p1=p2->next;
  while(p1 != nullptr){
    p2 -> next = p3;
    p3 = p2;
    p2 = p1;
    p1 = p1->next;
  }
  //p2 为结尾
  p2 ->next = p3;
  head->next = nullptr;
  head = p2;
}

void printLinkReversingly(const node* head){
    if(head == nullptr)
      return;
    printLinkReversingly(head->next);
    cout << head->info << endl;
}

void printLinkReversingly2(const node* head){
    if(head == nullptr) 
      return;
    stack<int> s;
    while(head != nullptr){
      s.push(head->info);
      head = head->next;
    }
    //print stack
}


//判断链表是否有环
bool isThereALoop(const node* head,int& len){
  if(head == nullptr || head->next == nullptr)
    return false;
  
  auto p1 = head;
  auto p2 = p1;
  while(p2 != nullptr){
    p2 = p2->next->next;
    p1 = p1->next;
    ++len;
    if(p1 == p2)
      return true;
  }
  return false;
}


//合并两个排序的链表
void merge(node* h1,node* h2,node*& h){
  if(h1 == nullptr || h2 == nullptr)
    return;
  h = (h1->info > h2->info) ? h2 : h1;
  auto p1 = h1;
  node* p1Next,*p2Next;
  auto p2 = h2;
  while(p1 && p2){
    if(p1->info < p2->info){
      p1Next = p1->next;
      p1->next = p2;
      p1 = p1Next;
    }else{
      p2Next = p2->next;
      p2->next = p1;
      p2 = p2Next;
    }
  }
}



int main(){
  node n1(1);
  node n2(2);
  node n3(3);
  node n4(4);
  node n5(5);
  node n6(6);
  node n7(7);
  n1.next = &n3;
  n2.next = &n4;
  n4.next = &n6;
  node* h;
  merge(&n2,&n1,h);
  printLink(h);
}
