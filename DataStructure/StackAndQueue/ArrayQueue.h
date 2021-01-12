


#pragma  once


#include "queue.h"
#include "myExceptions.h"
#include <sstream>


template<class T>
class ArrayQueue : public queue<T>{


  private:
    T* m_queue;               //基于数组实现的队列
    int m_length;
    int m_front,m_back;       //入队时:m_back+1
                              //出队时:m_front+1
                              //初始:m_back == m_front --> 队列为空
                              //满队时:(m_back+1) % m_length = m_front   ---> m_front和m_back在数组中可循环使用

  public:
    ArrayQueue(int capacity=10){
      if(capacity < 1){
        std::ostringstream s;
        s << "initial capacity = " << capacity << "must be > 0";
        throw illegalParameterValue(s.str());
      }
      m_length = capacity;
      m_queue = new T[m_length];
      m_front = m_back = 0;
    }

    ~ArrayQueue(){
      delete []m_queue;
    }

  public:
    bool empty() const override{
      return m_back == m_front;
    }


    //1:m_back > m_front ---> size = m_back - m_front
    //2:m_back < m_front ---> size = m_length-(m_back-m_front)
    //1和2整合有: (m_back-m_front+m_length) % m_length
    //设x=m_back-m_front 
    //size = x          ---> x>0
    //       m_length+x ---> x<0
    int size() const override{
      return (m_back - m_front + m_length) % m_length;
    }

    T& front() const override{
      if(empty())
        throw queueEmpty();
      //m_front本身并不指向队首部,m_front+1才是
      //比如起初m_front==m_back ---> 队列为空
      //入队一个元素 --> m_back+1 ---> m_front+1才指向队首元素
      //m_front用于判断队列是否已满
      return m_queue[(m_front+1) % m_length];  //有可能有m_front位于数组尾部
    }

    T& back() const override{
      if(empty())
        throw queueEmpty();
      return m_queue[m_back];
    }

    void pop() override{
      if(empty())
        throw queueEmpty();
      
      m_front = (m_front+1) % m_length;
      m_queue[m_front].~T();
    }

    void push(const T& element){
      if( (m_back+1) % m_length == m_front){
        T* new_queue = new T[2*m_length];

      }


    }

};
