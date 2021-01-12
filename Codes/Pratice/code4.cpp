

#include <iostream>
#include <string.h>
using namespace std;

//深拷贝和operator = 
class myStr{
  public:
    ~myStr(){
      delete  []str;
    }
    //copy ctor 
    myStr(const myStr& s){
      size = s.size;
      str = new char[size];
      strcpy(str,s.str);
    }
    myStr& operator=(const myStr& s){
      if(this != &s){
        delete []str;
        str = nullptr;

        size = s.size;
        str = new char[size];
        strcpy(str,s.str);
      }
      return *this;
    }
  private:
    char* str;
    int size;
};
//异常安全性的operator=
/*
 * myStr& operator=(const myStr& s){
 *    if(this != &s){
 *      myStr tmp(s);
 *      swap(s.str,str);
 *      size = s.size;
 *    }
 * }
 *
 * */
