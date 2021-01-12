



#include "../src/libgo/common/refsystem.h"


#include <iostream>
class X : public tiger::SharedRefCounter{

  public:
  ~X(){
    std::cout << "X is deleteed" << std::endl;
  }

};

void test(X* x){
  tiger::IncursivePtr<X> ix(x);
  std::cout << ix->get_count() << std::endl;
}

class A{

  private:
    void xx();
    int data;
};

class B : public A{

  void yy(){
    xx();
    ++data;
  }
};


int main(){
    X* x(new X);
    test(x);
    delete x;
    return 0;
}
