#pragma once
#include <memory>
#include <iostream>


namespace tiger{
 class Noncopyble{
  public:
    Noncopyble() =default;
    ~Noncopyble() = default;
    Noncopyble(const Noncopyble&) =delete ;
    Noncopyble& operator=(const Noncopyble& ) =delete ;
 };

 

//初始化生成单例
template<class T>
class Singleton{
  private:
    Singleton(){}
  public:
    Singleton(const Singleton& ) = delete ;
    Singleton(Singleton&& ) = delete ;
    static T* GetInstance(){
      static T v;
      return &v;
    }
};

template<class T>
class SharedSingleton{
  public:
    SharedSingleton(const SharedSingleton& ) = delete ;
    SharedSingleton(SharedSingleton&& ) = delete ;
    static std::shared_ptr<T> GetInstance(){
      static std::shared_ptr<T>v(new T());
      return v;
    }
  private:
    SharedSingleton(){}
};
}

