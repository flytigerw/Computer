


#include <iostream>
#include <sstream>
#include "myExceptions.h"
using namespace std;


//字符串相关的问题


//将字符串转为整数 ---> "890\0"  
//顺序遍历，解析每个字符.
//核心规则: num = num*10+new_num;
//要支持+,-符号检测
int stoi(const char* s){
  if(s == nullptr)
    throw illegalParameterValue("string must be null");
  int num = 0;
  bool is_sign_check = false;
  bool pos = false;
  while(*s != 0){
    //非数字
    if(*s < '0' || *s > '9'){
      //第一判断正负号
      if(!is_sign_check){
          if(*s == '+')
            pos = true;
          else 
            pos = false;
          is_sign_check = true;
      }else{ //后续为非法字符
         stringstream sout;
         sout << "含有非数字字符:" << *s;
         throw illegalInputData((sout.str()));
      }
    }else  //数字
      num = num*10 + (int)(*s-'0');
    ++s;
 }
  return pos ? num : -num;
}

void stoi_test(){
  cout << "1234 ---" <<stoi("1234") << endl;
  cout << "-1234 ---" <<stoi("-1234") << endl;
  cout << "-1a2-34 ---" <<stoi("-1a2-34") << endl;
}


int main(){
  stoi_test();
  return 0;
}
