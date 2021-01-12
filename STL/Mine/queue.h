

#ifndef QUEUE
#define QUEUE 


#include "deque.h"
#include "iterator.h"
#include "vector.h"
#include "functional.h"
#include "heap_algorithm.h"
namespace stl{
  template<class T,class Container=deque<T>>
  class queue{
    friend bool operator==(const queue&,const queue&);
    friend bool operator<(const queue&,const queue&);
    
    public:
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;

    protected:
    Container c;
    public:
    bool empty() const{
      return c.empty();
    }
   size_type size() const { return c.size(); }
   reference front() { return c.front(); }
   const_reference front() const { return c.front(); }
   reference back() { return c.back(); }
   const_reference back() const { return c.back(); }
   void push(const value_type& x) { c.push_back(x); }
   void pop() { c.pop_front(); }
  };
template <class T, class Sequence>
bool operator==(const queue<T, Sequence>& x, const queue<T, Sequence>& y) {
  return x.c == y.c;
}

template <class T, class Sequence>
bool operator<(const queue<T, Sequence>& x, const queue<T, Sequence>& y) {
  return x.c < y.c;
}





//优先队列
template<class T,class Container = vector<T>,class Compare = less<typename Container::value_type>>
class priority_queue{
  
  //容器所含类型定义
  public:
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type  size_type;
    typedef typename Container::reference   reference;
    typedef typename Container::const_reference const_reference;
  protected:
    Container c;
    Compare comp;
  public:
    priority_queue():c(){}  //调用底层容器的默认构造函数
    explicit priority_queue(const Compare&x):c(),comp(x){}
    //采用迭代器范围进行初始化
    //先用迭代器范围对底层容器初始化，在这一部分数据建成堆
    template<class InputIterator>
      priority_queue(InputIterator first,InputIterator last,const Compare&x):c(first,last),comp(x){
        make_heap(c.begin(),c.end(),comp);
      }
    template<class InputIterator>
      priority_queue(InputIterator first,InputIterator last):c(first,last){
        make_heap(c.begin(),c.end(),comp);
      }
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    const_reference top() const { return c.front(); }

    void push(const value_type& x){
      try{
          c.push_back(x);   //可能抛出异常
          rebuild_heap_for_push(c.begin(),c.end(),comp);
      }catch(...){
        c.clear();
        throw;
      }
    }
    void pop(){
      try{
        rebuild_heap_for_pop(c.begin(),c.end(),comp);
        c.pop_back();
      }catch(...){
        c.clear();
        throw;
      }
    }
};

}
#endif
