


#include <iostream>
using namespace std;

//二分查找
template<class T>
bool binSearch(T elements[],int beg,int end,T ele){
  if(elements == nullptr || beg > end)
    return false;
  int mid;
  while(beg<=end){
    mid = (end+beg) / 2;
    if(elements[mid] == ele)
      return true;
    else if(elements[mid] > ele)
      end = mid - 1;
    else 
      beg = end + 1;
  }
  return false;
}

//旋转数组的查找
bool findInRotate(int numbers[],int beg,int end,int x){
  if(numbers == nullptr || beg > end)
    return false;

  int mid = (end+beg) / 2;
  if(numbers[beg] == x || numbers[end] == x || x == numbers[mid])
      return true;
  
  if(numbers[mid] == numbers[beg]){ //顺序查找
      for(int i=beg;i<end;i++)
        if(numbers[i] == x)
          return true;
      return false;
  }

  if(numbers[mid] < numbers[beg]){ //mid位于r2中
      if(x > numbers[mid] && x < numbers[end])         //在(mid,end)中二分查找
        return binSearch(numbers,mid+1,end-1,x);
      else 
        return findInRotate(numbers,beg+1,mid-1,x);
  }else{    //mid位于r1中
    if(x < numbers[mid] && x > numbers[beg])
        return binSearch(numbers,beg+1,mid-1,x);
    else 
        return findInRotate(numbers,mid+1,end-1,x);  
  }
   return false;
};


int main(){
  int d[] = {4,5,6,7,8,9,10,2,2,2,0};
  cout << findInRotate(d,0,10,2)<< endl;
  return 0 ;
}
