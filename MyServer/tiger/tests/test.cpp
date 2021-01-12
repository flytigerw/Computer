

#include <iostream>
#include<vector>
#include <string>
#include <tuple>
using namespace std;


int m_error = 0;
void parse_pattern(const string& name){
  //0 ----> dwadw
  //1 ----> %x
  //2 ----> %d{%yyy}
  std::vector<tuple<int,std::string,std::string>> v;
  int s = 0;
  char c;
  string normal_str;
  string fmt,extra_fmt;
  //进入状态时要保证i始终指向当前字符
  for(size_t i=0;i<name.size();){
    c = name[i];
    switch(s){
      case 0:
        if(name[i] == '%')
          s = 1;
        else 
          s= 3;
        break;
      case 1:
        ++i;  //现在i指向%的下一个
        if(i<name.size())
          fmt.push_back(name[i]);
        ++i;
        if(name[i] == '%'){
          v.push_back(make_tuple(0,fmt,""));
          fmt.clear();
          s = 0;
        }else if(name[i] == '{')
          s = 2;
        else{
          v.push_back(make_tuple(1,fmt,""));
          fmt.clear();
          s = 0;
        }
        break;
      case 2:
        ++i;  //现在i指向{的下一个
        while(i<name.size() && name[i] != '}')
          extra_fmt.push_back(name[i++]);
        if(name[i] != '}')
          m_error = 1;
        else
          v.push_back(make_tuple(2,fmt,extra_fmt));
        fmt.clear();
        extra_fmt.clear();
        s = 0;
        ++i;  //现在i指向}的下一个
        break;
      case 3:
        while(i<name.size() && name[i] != '%')
          normal_str.push_back(name[i++]);
        v.push_back(make_tuple(0,normal_str,""));
        normal_str.clear();
        s = 0;
        break;
    }
  }
  for(const auto& i:v)
    cout << get<0>(i) <<"--" << get<1>(i) << get<2>(i) << endl;
}

int main(){
  string s= "%d{%Y-%m-%d %H:%M:%S}%T{%t}%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
  parse_pattern(s); 
}
