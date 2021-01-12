


#pragma once 

#include <memory>

template<class K,class V>
class dictionary{


  public:
      virtual ~dictionary() {}
      virtual bool empty() const = 0;
      virtual int size() const = 0;
      //根据K进行增删查
      virtual std::pair<const K, V>* find(const K&) const = 0;
      virtual void erase(const K&) = 0;
      virtual void insert(const std::pair<const K, V>&) = 0;
};
