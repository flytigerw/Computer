


#include <iostream>
using namespace std;

template<class T>
int partition(T list[],int low,int high){
  if(low == high)
    return low;
  int mid = (low+high) / 2;
  T tmp = list[mid];
  swap(list[mid],list[high]);
  int small = low-1;
  int i;
  for(i=low;i<high;i++)
    if(list[i] <= tmp)
      swap(list[small++],list[i]);
  swap(list[high],list[mid]);
  return mid;
}
template<class T>
T __getMoreThanHalf(T list[],int low,int high,int mid){
  int idx = partition(list,low,high);
  if(idx == mid)
    return list[mid];
  else if(idx > mid)
    return  __getMoreThanHalf(list,low,idx-1,mid);
  else 
    return __getMoreThanHalf(list,idx+1,high,mid);
}

template<class T>
void getMoreThanHalf(T list[],int low,int high,T& x){
  if(list == nullptr || low > high)
    return;
  int mid = (high+low) / 2;
  x = __getMoreThanHalf(list,low,high,mid);
}

template<class T>
void getMoreThanHalf2(T list[],int size,T& result){
  if(list == nullptr || size < 0)
    return;
  T x = list[0];
  int count = 1;
  for(int i=1;i<size;i++){
    if(list[i] != x && --count == 0)
      x = list[i];
    ++count;
  }
  result = x;
}

int main(){
  int a[] = {1,2,3,2,2,2,5,4,2};
  int x = 0;
  getMoreThanHalf2(a,9,x); 
  cout << x << endl;
}
