

#include <iostream>

using namespace std;



//计算一个整数中1的个数
int calculate1s(int num){
  int res = 0;
  int x = 1;
  while(x != 0){
    if((num & x) != 0)
      ++res;
    x <<= 1;
  }
  return  res;
}

//不断地剔除1
int calculate1s2(int num){
    int res;
    for(res =0;num != 0;res++)
        num = (num-1) & num;
    return res;
}


//判断一个数是否为2的整数次幂
bool isPowerOf2(int num){
  return ((num-1) & num) == 0;
}
int main(){
  cout << isPowerOf2(32) << endl;
}


