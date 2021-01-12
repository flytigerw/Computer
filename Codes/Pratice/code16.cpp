
#include <iostream>
using namespace std;

bool equal(double n1, double n2){

}


double powerCore(double base,int ex){
     //递归边界:ex为1
     if(ex == 1)
       return base;
     if((ex & 1)  == 0)
       return powerCore(base,ex>>2) * powerCore(base,ex>>2);
     else 
       return powerCore(base,(ex-1)>>2) * powerCore(base,(ex-1)>>2) * base;
}

double power(double base,int ex){
   //参数检查
   if(equal(base,0.0))
     return 0;
   //结果的正负
   bool positive = true;
   if(base < 0 && ex % 2 == 1)
     positive = false;
   
   //结果是否为倒数
   bool upToDown = (ex < 0) ? true : false;
   base = (base < 0 )? -base : base;
   ex = (ex < 0 ) ? -ex : ex;

   double res = powerCore(base,ex);
   if(!positive)
     res = -res;
   if(upToDown)
     res = 1/res;
   return res;
}

int main(){
  return  0;
}
