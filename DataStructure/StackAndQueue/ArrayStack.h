


#pragma once 
#include "stack.h"
#include "myExceptions.h"
#include "myArray.h"
#include <sstream>


template<class T>
class ArrayStack : public stack<T>{


  private:
    int m_top;
    int m_length;   
    T* m_stack;           //基于数组实现的stack

  public:
    ArrayStack(int capacity=10){
      if(capacity < 1){
        std::ostringstream s;
        s << "initial Capacity = " << capacity << "Must be > 0";
        throw illegalParameterValue(s.str());
      }
      //分配空间并初始化
      m_length = capacity;
      m_stack = new T[m_length];
      m_top = -1;          //empty
    }
    ~ArrayStack(){
      delete [] m_stack;
    }

  public:

    bool empty() const override{
      return m_top = -1;           
    }

    int size() const override{     //important
      return m_top+1;
    }

    T& top() override{
      if(m_top == -1)
        throw stackEmpty();
      return m_stack[m_top];
    }

    void pop() override{
      if(m_top == -1)
        throw stackEmpty();
      //调用析构函数进行元素销毁
      m_stack[m_top--].~T();
    }

    void push(const T& element) override{
      //检查是否需要扩容
      if(m_top == m_length-1){
        change1Dlength(m_stack,m_length,m_length*2);
        m_length *= 2;
      }

      m_stack[++m_top] = element;
    }

};

