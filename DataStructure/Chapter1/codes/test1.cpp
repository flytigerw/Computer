#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

//容器类:数据类型+大小 ---> 推迟到对象的定义阶段
template<class Type,int size=50>
class Container{
   Type container[size];
};

//交换两个数值的标准操作
template<class Type>
void swap(Type& el1,Type& el2){
  Type tmp=el1;
  el1=el2;
  el2=tmp;
}

//数据成员为指针时，类应该包含三剑客
class Node{
  public:
    //copy ctor 
    Node(const Node& n){
      name = strdup(n.name);
      age=n.age;
    }
    //dctor
    ~Node(){
      free(name);
    }
    //copy =
    Node& operator=(const Node& n){
      if(this != &n){  //No assignment to self
        if(name != nullptr)
          free(name);     //搭配strdup使用
          name = strdup(n.name);
          age = n.age;
      }
      return *this;  //返回可修改的左值
    }

  private:
    int age;
    char* name;
};


//求和函数
//参数:上下限以及对求和之前对元素的处理方式
template<class Process,class Element>
Element sum(Process p,Element low,Element high){
  Element reuslt;
  for(Element i=low;i<=high; i++)
    reuslt += p(i);
  return reuslt;
}
//sum(-,1,10)
//-为一种处理方式.用函数对象来表达
template<class T>
struct neg{
  T operator()(const T& ele) const{
    return -ele;
  }
};

//二分法求函数的根
//参数:区间两端，最小精度，函数
//不断通过二分法缩小区间直到最小精度
double root(int(* func)(double),double left,double right,double eplison){
        double middle = (left+right)/2;
        while((func(middle != 0)) && (fabs(left-right)>eplison)){
                //二分缩小区间
                if(func(left)*func(middle) < 0)
                  right = middle;
                else left = middle;
                middle = (left+right)/2;
        }
        return middle;
}

//异常处理示例
int sum(int x,int y){
     if(x <= 0 || y <= 0)
       throw "All parematers should be>0";        //char* 异常
     return x+y;
}
void exceptionTest(int x,int y){
  try{
      cout << sum(x,y) << endl;
  }catch(char* e){
      cout << e << endl;
  }
}


int main(){
  cout << sum(neg<int>(),1,10) << endl;
  return 0;
}
