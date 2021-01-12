

#include <iostream> 
#include <iterator>
using namespace std; 


template<class T>
void permutations(T list[],int k,int m){
  //输出list[k:m]的所有排列
  static int s = k;
  //递归边界
  //k==m--->只有一个元素
  if(k == m){
    copy(list+s,list+m+1,ostream_iterator<T>(cout," "));
    cout << endl;
  }
  else{
    for(int i=k;i<=m;i++){
      swap(list[k],list[i]);          
      permutations(list,k+1,m);            //k+1 
      swap(list[i],list[k]);
    }
  }
}

//Greatest Common Divisor
int gcd(int x,int y){
  if(y>x)
    swap(x,y);
  if(y == 0)
    return x;
  return gcd(y,x%y);
}
int main(){
  cout << gcd(17,9)  << endl;
  return 0;
}
