

#include <iostream>
#include <string.h>
using namespace std;

inline
int addChars(char c1,char c2,char& c){
    // 0-48  9-57
    int x = c1+c2;
    if(x >= 106){ //发生进位
      c =(char)(x - 106);
      return 1;
    }
    c = (char)(x-48);
    return 0;
}

//调整数组，让奇数位于偶数前面
void ajustArray(int nums[],int size){
  //参数检查..
  int w = -1;
  for(int i=0;i<size;i++)
    if((nums[i] & 1) != 0)
      swap(nums[++w],nums[i]);

  for(int i=0;i<size;i++)
    cout << nums[i] << ' ';
  cout << endl;
}
int main(){
  int a[] = {2,4,5,6,8,9,1};
  ajustArray(a,7);
  return 0;
}
