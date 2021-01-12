


#include <iostream>
#include <vector>
using namespace std;


//长度为n的数组中，所有数字都在0-n-1范围中
//找出数组中任意重复的数字
//思路:重复 ----> 遍历进行建立hash表，key值为数字，在0 - n-1范围中 ----> 直接以数组下标为索引
void repeat_number(int list[],int size){
  if(list == nullptr || size <= 0)
    return;
  vector<int> m_hash(size,0);
  for(int i=0;i<size;++i){
    if(m_hash[list[i]] == 0)
      ++m_hash[list[i]];
    else{
      cout << list[i] << endl;
    }
  }
}

//不采用额外空间进行记录 -----> 在原数组上进行记录  
//设 x = list[i] -----> 在哪记录x呢 ?
//用list[x]来记录x已经出现
//当再遇到x = list[j]时，此时发现list[x] == x ---> 已经出现过

void repeat_number2(int list[],int size){
  if(list == nullptr || size <= 0)
    return;
  int x;
  for(int i=0;i<size;i++){
    x = list[i];

    if(x != i){
     if(list[x] == x){     //已经出现过
       cout << x << endl;
       break;
     }else swap(list[x],list[i]);    //此时list[x] = x ---> 记录x出现过
    }

  }
}

void repeat_number_test(){
  int list[6] = {0,2,3,4,1,1};
  repeat_number2(list,6);
}


int main(){
  repeat_number_test();
}
