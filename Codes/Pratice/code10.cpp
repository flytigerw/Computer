



#include <iostream>
using namespace std;


long long fibnacci(long long n){
  if(n < 0)
    return n;
  if(n == 0)
    return 0;
  if(n == 1)
    return 1;
  
  long long i = 2;
  long long f3, f2 = 1,f1 = 0;
  while(i <= n){
     f3 = f2 + f1;
     //下一轮
     f1 = f2;
     f2 = f3;
     i++;
  }
  return f3;
}

int main(){
  for(int i = 0;i<20;i++)
    cout << fibnacci(i) << endl;
  return 0;
}
