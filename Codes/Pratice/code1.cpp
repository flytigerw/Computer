#include <iostream>
#include "myExceptions.h"
#include <sstream>
using namespace std;

//stoi 
int stoi(const char* str){
  if(str == nullptr)
    throw illegalParameterValue("it must be a non-nullptr");
  int num;
  bool pos = true;
  bool signCheck= false;
  while(*str != 0){
    //非数字字符检查
    if(*str < 48|| *str > 57){
      if(!signCheck){
       //正负号检查
       if(*str == '-')
         pos = false;
       signCheck = true;
       ++str;
       continue;
      }
      else{
       stringstream s;
       s << "含有非数字字符: " << *str;
       throw illegalInputData(s.str());
      }
    }
    num = num*10 + (*str-'0');
    ++str;
  }
  return pos ? num : -num;
}


int main(){
  try{
   cout << stoi("-123a") << endl;
  }catch(illegalInputData x){
    x.outputMessage();
  }
}
