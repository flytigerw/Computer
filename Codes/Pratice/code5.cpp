


#include <iostream>
#include <pthread.h>
using namespace std;

//懒汉式 ---> 加载类代码时就创建实例 ---> 线程安全
template<class T>
class singleton1{
  public:
    singleton1<T>(const singleton1<T>& ) = delete ;
    singleton1<T>& operator=(const singleton1<T>&) = delete;
    static T* getInstance(){
      return myInstance;
    }
    static void  destoryInstance(){
          delete myInstance;
          myInstance = nullptr;
    }
  private:
    singleton1(){}
    ~singleton1(){}
    static T* myInstance;
};
template<class T>
T* singleton1<T>:: myInstance = new T();


//饿汉式 -----> 需要时生成静态实例 -----> 线程不安全 ----> 需要加锁同步,保证创建实例时的互斥性 ----> 第一个实例创建后，后续调用不用再进行mutex操作

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

template<class T>
class singleton2{
  public:
    singleton2<T>(const singleton2<T>& ) = delete ;
    singleton2<T>& operator=(const singleton2<T>&) = delete;
    
    //创建实例时，传递可变参数
    template<class... Args>
      static T* getInstance(Args&&... args){
        //避免实例创建以后的mutex操作
        if(myInstance == nullptr){
          pthread_mutex_lock(&mutex);
          //多个线程都调用getInstance()第一次创建实例时需要保证:1.互斥性 2.创建以后不再创建
          if(myInstance == nullptr){
            myInstance = new T(forward<Args>(args)...);         
          }
          pthread_mutex_unlock(&mutex);
        }
        return  myInstance;
      }
  private:
    static T* myInstance;
};

template<class T>
T* singleton2<T>::myInstance = nullptr;
