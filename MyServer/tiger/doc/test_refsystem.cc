

#include <iostream>
#include "refsystem.h"



class A : public tiger::RefCounter{

  public:
    ~A(){
      std::cout << "A is deleted" << std::endl;
    }

};

int main(){
  tiger::IncursivePtr<A> p1(new A);
}
