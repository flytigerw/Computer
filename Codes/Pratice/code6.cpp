


#include <iostream>
#include <unistd.h>
#include "myArray.h"
using namespace std;

bool duplicate(int numbers[],int size,int* dup){
  //参数检查
  if(numbers == nullptr || size <= 0)
    return false;
  for(int i=0;i<size;i++){
    if(numbers[i] == i)
      continue;
    if(numbers[numbers[i]] == numbers[i]){
      *dup = numbers[i];
      return true;
    }
    swap(numbers[i],numbers[numbers[i]]);
  }
  return false;
}

void duplicate2(int numbers[],int size,int* res){
    //用数组建立散列表
  for(int i=0;i<size;i++)
    res[numbers[i]]++;
}

int countRange(const int numbers[],int size,int beg,int end){
  int count = 0;
  for(int i=0;i<size;i++){
    if(numbers[i] >= beg && numbers[i] <= end)
      ++count;
  }
  return count;
}

//不能改变原数组,也不能进进行
bool duplicate3(const int numbers[],int size,int* dup){
    //简单题，进行参数检查
    if(numbers == nullptr || size <= 0)
      return false;
    int beg = 1;
    int end = size-1;
    int mid = 0;
    int count = 0;
    while(beg <= end){
       mid = (beg+end) / 2;
       count = countRange(numbers,size,beg,mid);
       
       if(beg == mid){
         //在右半边找到
         if(count <= 1)
           ++beg;
         break;
       }
         
       if(count > (mid-beg+1))
         end = mid;
       else
         beg = mid+1;
    }
    *dup = beg;
    return true;
}

using t = int (*) [4];
bool FindIn2DArray(t matrix,int rows,int cols,int num){
  if(matrix == nullptr || rows<=0 || cols<=0 )
    return false;
  int rx = 0,ry = cols - 1;
  while(rx <rows && ry >= 0){
    if(matrix[rx][ry] == num)
      return true;
    if(matrix[rx][ry] > num){
      --ry;
    }else{
      ++rx;
    }
  }
  return false;
}

//字符串的替换
//len 为字符串的最大容量
void replaceBlank(char* str,int len){
  if(str == nullptr || len<= 0)
    return;
  
  int blanks = 0;
  int size = 0;
  while(str[size++] != 0)
    if(str[size] == ' ')
      ++blanks;
  int newSize = size+2*blanks;

  if(newSize >= len)
    return;

  int p1 = size - 1,p2 = newSize - 1;
  while(p1 >= 0){
    if(str[p1] != ' '){
      str[p2--] = str[p1];
    }else{
      str[p2--] = '0';
      str[p2--] = '2';
      str[p2--] = '%';
    }
     p1--;
  }
}




int main(){
  int a[] = {2,3,1,0,2,5,3};
  int b[] = {2,3,5,4,3,2,6,7};
  int c[4][4] = {
    {1,2,8,9},
    {2,4,9,12},
    {4,7,10,13},
    {6,8,11,15},
  };

  int res[7]={0};
  int x = 0;
  duplicate2(a,7,res);
  for(auto i : res)
  cout << i << ' ';
  cout << endl;
  cout << duplicate(a,7,&x) << "--"<<x << endl;
  cout << duplicate3(b,8,&x) << "--" << x << endl;
  cout << FindIn2DArray(c,4,4,10) << endl;
}
