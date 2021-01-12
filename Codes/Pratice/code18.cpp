




#include <iostream>
#include <vector>
#include <unistd.h>
using namespace std;

template<class T>
inline
void printVector(vector<T>& v){
  for(const auto& x : v)
    cout << x;
  cout << endl;
}


template<class T>
void combinations_impl(T list[],int low,int high,vector<T>& v){
  if(low > high)
    return;
  //每一层的区域 [low,high]
  //不断压入区域中的每一个元素并进入相应的下一子区域 
  //若下一区域非法，则打印路径
  for(int i=low;i<=high;i++){
    v.push_back(list[i]);
    combinations_impl(list,i+1,high,v);
    printVector(v);
    v.pop_back();
  }
}

template<class T>
void combinations(T list[],int low,int high){
  if(low > high)
    return;
  vector<T> v;
  v.push_back(list[low]);
  combinations_impl(list,low+1,high,v);
  for(int i =low;i <= high; i++)
    cout << list[i] << ' ';
  cout << endl;
}

struct pos{
  int x,y;
};

bool checkElement(vector<pos>& v,const pos& p){
      if(v.empty())
        return true;
      for(const auto& i : v)
        if(i.x == p.x || i.y == p.y || abs(p.x-i.x) == abs(p.y - i.y))
          return false;
      return true;
}
void queenChess(int deep,int size,vector<pos>& v){
  if(deep == size){
   for(const auto& i : v)
     cout << i.x << "-" << i.y << ' ';
   cout << endl;
    return;
  }
  //每一层的区域大小:[0,size) 
  //选择每一个元素，看是否不与前面的元素冲突
  for(int i=0;i<size;i++){
    if(checkElement(v,{deep,i})){
      v.push_back({deep,i});
      queenChess(deep+1,size,v);
      v.pop_back();
    }
  }
}
void queenChess(int size){
  if(size < 4)
    return;
  vector<pos> v;
  queenChess(0,size,v);  
}
int main(){
  char x[] ={'a','a','b'};
  combinations(x,0,2);
  return 0;
}
