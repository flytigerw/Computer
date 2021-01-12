


#include <iostream>
using namespace std;


class empty{

};
class emptyWithVirtual{
  virtual ~emptyWithVirtual();
};
class emptyWithStatic{
  static int i;
};
int emptyWithStatic::i = 1;
/* sizeof() ---> 类型的一个实例占用的内存大小
 *
 * 1.空类型，没有任何变量和成员函数
 *   类的声明需要在内存中占有一定的空间 
 *   至于要分配多少空间则由编译器决定
 * 2.空类型+构造和析构函数 或者 static
 *   函数的地址只与类的类型有关，由编译器维护二者的联系.与实例无关
 * 3.空函数+虚函数
 *   编译器为类的类型生成虚函数表
 *   类的每个实例都会有一个指向虚函数表的虚指针
 *   故结果为虚指针的大小
 * */

int main(){
  cout << "empty: " << sizeof(empty) << endl; 
  cout << "emptyWithVirtual: " << sizeof(emptyWithVirtual) << endl; 
  cout << "emptyWithStatic: " << sizeof(emptyWithStatic) << endl; 
}
