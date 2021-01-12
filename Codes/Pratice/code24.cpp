



#include <iostream>
#include <limits>
#include <vector>
#include <unistd.h>
#include <list>
using namespace std; 


void probability_impl(int depth,int last,int n,int k,vector<int>&s){
  if(depth == n){
    for(int i=1;i<=k;i++)
      ++s[i+last];
    return;
  }
  for(int i=1;i<=k;i++)
    probability_impl(depth+1,last+i,n,k,s);
}

//n个骰子，每个骰子有k点
void probability(int n,int k){
  if(n < 1 && k < 1)
    return;
  vector<int> s(n*k+1,0);
  probability_impl(1,0,n,k,s);
  for(int i=0;i<n*k+1;++i){
    cout << i << "-" << s[i] << endl;
  }
}


void probability2(int n,int k){
  if(n < 1 && k < 1)
    return;
  int s[2][n*k+1] ;
  for(int i=0;i<2;i++)
    for(int j=0;j<n*k+1;++j)
      s[i][j] = 1;
  int turn = 0;

  //顺序扔出每个骰子
  //扔到第x个骰子时，可能出现的点数和范围:[x,xk]
  //前一个骰子的点数和范围[x-1,(x-1)k]
  for(int i=1;i<=n;i++){

    for(int j=i;j<=i*k;j++)   //清除之前骰子数据的遗留
      s[turn][j] = 0;

    for(int j=i-1;j<=(i-1)*k;j++){   //前一个骰子
      for(int m=1;m<=k;++m)         //前一个骰子的每个点数和可以为下一个骰子中k个点数和做贡献
        s[turn][j+m] += s[1-turn][j];  
    }

    turn = 1-turn;
 }

  for(int i=0;i<n*k+1;i++)
    cout << i << "-" << s[1-turn][i] << endl;
}

template<class T>
void last_num(T a[],int size,int m){
  if(a == nullptr || size < 1)
    return;
 
  list<T> l(a,a+size);
  auto iter = l.begin();
  for(int i=0;i<size-1;i++){
    for(int j=0;j<m-1;j++)
      if(++iter == l.end())
        iter = l.begin();
    cout << *iter << endl;
    l.erase(iter++); 
    if(iter == l.end())
      iter = l.begin();
  }
}

template<class T>
void last_num2(T a[],int size,int m){
  if(a == nullptr || size < 1)
    return;
  int last = 0;
  for(int i=2;i<=size;i++)
    last = (last + m) % i;
    cout << a[last] << endl;
}

void min_max(int* list,int size){
  if(list == nullptr || size < 1)
    return;
  int min_idx = 0,max_idx = 0;
  auto min = numeric_limits<int>::max();
  auto max = numeric_limits<int>::min();
  for(int i=0;i<size;i++){
    if(list[i] < min){
      min = list[i];
      min_idx = i;
    }else if(list[i] > max){
      max = list[i];
      max_idx = i;
    }
  }
  cout << min << "--" << max << endl;
}
int main(){
  int a[] = {0,1,7,3,4};
  min_max(a,5);
  return 0;
}
