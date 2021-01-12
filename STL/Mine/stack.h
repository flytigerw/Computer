

#ifndef STACK 
#define STACK 

#include "deque.h"
namespace stl{
  //默认采用deque作为底层容器
  template<class T,class Container = deque<T>>
    class stack{
        friend bool operator==(const stack&,const stack&);
        friend bool operator<(const stack&,const stack&);
      public:
        typedef typename Container::value_type value_type;
        typedef typename Container::size_type  size_type;
        typedef typename Container::reference  reference;
        typedef typename Container::const_reference const_reference;

      protected:
        Container c;
      public:
        bool empty() const {
          return c.empty();
        }

        size_type size() const {
          return c.size();
        }

        reference top(){
          return c.back();
        }
        const_reference top() const{
          return c.back();
        }
        void push(const value_type& x){
          c.push_back(x);
        }
        void pop(){
          c.pop_back();
        }
    };
template <class T, class Sequence>
bool operator==(const stack<T, Sequence>& x, const stack<T, Sequence>& y) {
  return x.c == y.c;
}

template <class T, class Sequence>
bool operator<(const stack<T, Sequence>& x, const stack<T, Sequence>& y) {
  return x.c < y.c;
}
}

#endif
