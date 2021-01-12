

#include <iostream>
#include <unistd.h>
using namespace std; 



//快速排序
template<class T>
int partition(T elements[],int beg,int end){
     int mid = (end+beg) / 2;
     swap(elements[mid],elements[end]);
     int small = beg - 1;
     for(int i= beg; i<end; i++){
       if(elements[i] < elements[end])
          swap(elements[++small],elements[i]);
     }
     swap(elements[small+1],elements[end]);
     return small+1;
}

//不进行错误检查
template<class T>
void quickSort(T elements[],int beg,int end){
  if(beg >= end)
    return;
  int idx = partition(elements,beg,end);
  quickSort(elements,beg,idx-1);
  quickSort(elements,idx+1,end);
}

int main(){
  int a[] = {1,3,4,6,10};
  int b[] = {8,5,9,6,2,4,1};
  quickSort(b,0,6);
  for(auto i : b)
    cout << i << ' ';
  cout << endl;
  cout << binSearch(a,0,4,4) << endl;
}
