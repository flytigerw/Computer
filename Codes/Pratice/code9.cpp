

#include <iostream>
#include <stack>
#include <queue>
using namespace std;

//用两个栈实现队列
template<class T>
class myQueue{
  public:
    void push(const T& element){
      s1.push(element);
    }

    void pop(){
      if(s2.empty())
        transfer();
      s2.pop();
    }

    T top() const{
      if(s2.empty())
        transfer();
      return  s2.top();
    }

  private:
    stack<T> s1;
    stack<T> s2;
    void transfer(){
        while(!s1.empty()){
          s2.push(s1.top());
          s1.pop();
        }
      }
};
    
//用两个队列实现栈
template<class T>
class myStack{
  public:
    void push(const T& element){
        (q1.empty()) ? q2.push(element) : q1.push(element);
        stackHead = element;
    }

    void pop() {
      if(q2.empty()){
        while(true){
          q2.push(q1.front());
          q1.pop();
          if(q1.empty()){
            q2.pop();
            return;
          }
        }
      }
    }

    void top() const {return stackHead;}
  private:
    queue<T> q1;
    queue<T> q2;
    T stackHead;
};


//包含getMin()函数的栈
template<class T>
class minStack{
  public:
    void push(const T& element){
      if(!min.empty() && element > min.top())
        s.push(element);
      else {
        s.push(element);
        min.push(element);
      }
    }
    
    T top() const {
      if(s.empty()){

      }
        ///..错误处理
      else 
        return s.top();
    }

    T getMin() const{
      if(min.empty()){
        //错误处理
      }else 
        return min.top();
    }
    
    void pop() {
      if(s.empty())
        return;
      if(s.top() == min.top()){
        s.pop();
        min.pop();
      }else s.pop();
    }
  private:
    stack<T> s;
    stack<T> min;
};
