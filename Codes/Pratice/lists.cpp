

#include <iostream>
using namespace std;

//链表类相关题目

template<class T>
struct slist_node_base{
  slist_node_base* next;
  T data;
  slist_node_base(const T& d) : data(d),next(nullptr){}
};

//求单向链表中倒数第K个节点
//1)用一个指针，先求出链表节点数量X，再前进X-K个 ---> 两边扫描
//2)用两个指针，保持二者有K个身位差,其中一个到终点时，另一个就是倒数第K个
template<class T>
slist_node_base<T>* find_kth_node(slist_node_base<T>* root,int k){
    if(root == nullptr)
      return nullptr;
    auto first = root;
    auto second = first;
    //first先前进k步
    while(k-- > 0){
      first = first->next;
      //有可能链表不够长,first先行已经到达终点
      if(first == nullptr)
        return nullptr;
    }
    for(;first;first=first->next,second=second->next);
    return second;
}

void find_kth_node_test(){
  slist_node_base<int> n1(1);
  slist_node_base<int> n2(2);
  slist_node_base<int> n3(3);
  slist_node_base<int> n4(4);
  slist_node_base<int> n5(5);
  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;
  n4.next = &n5;
  auto result1 = find_kth_node(&n1,2);
  cout << result1->data << endl;
  auto result2 = find_kth_node(&n1,6);
  if(!result2)
    cout << "不存在" << endl;
}


int main(){
  find_kth_node_test();
  return 0;
}
