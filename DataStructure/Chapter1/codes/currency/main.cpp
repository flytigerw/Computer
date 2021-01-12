

#include "currency.h"
#include <iostream>
using namespace std;
int main(){
  //测试构造函数和setValue以及<<
  Currency c1(2.12),c2(-22.11),c3(positive,10,90);
  cout << c1 << c2 << c3 << endl;
  //测试+与+=
  Currency c = c1+c2;
  cout << c << endl;
  cout << (c1+=c2+=c3) << endl;
}
