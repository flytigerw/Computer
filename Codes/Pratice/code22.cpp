

#include <iostream>
#include <unistd.h>
using namespace  std;

int get_first1_idx(int x){
  int i;
  int m=1;
  for(i=0;(x & m) == 0 && m<=x;i++,m <<=1);
  return i;
}

template<class Predicate>
int partition(int* nums,int size,Predicate pred){
  int first = 0;
  int last = size;
  while(true){
    while(pred(nums[first]))
      ++first;
    --last;
    while(!pred(nums[last]))
      --last;
    if(first >= last)
      return last;
    std::swap(nums[first],nums[last]);
    ++first;
  }
}
void find_nums_appear_once(int* nums,int size){
  if(nums == nullptr || size<2)
    return;
  int x = nums[0];
  //先进行异或运算
  for(int i=1;i<size;i++)
    x ^= nums[i];
  //找到x中第一个1的位置 ---> 第一个不同点
  int n = get_first1_idx(x);
  int y = 1 << n;
  //进行分组
  int mid = partition(nums,size,[&](int d){
      return (d & y) == 0;
      });
  for(int i=0;i<size;i++)
    cout << nums[i] << ' ';
  cout << endl;
  cout << mid << endl; 
}


//获取一个数字的第i位
int get_bit(int n,int i){
  int x = 1 << i;
  return ((n & x) == 0) ? 0 : 1;
}

void find_nums_appear_once2(int* nums,int size){
  if(nums == nullptr || size<1)
    return; 
  int sum[32] ={0};
  //累加每一个数字的每一位
  int x;
  for(int i=0;i<size;i++){
    x = 1;
    for(int j=0;j<32 && (x << j)<= nums[i];j++)
        sum[j] += get_bit(nums[i],j);
  }
  for(auto i:sum)
    cout << i << ' '; 
  cout << endl;
}

void find_continuous_sequence(int n){
  if(n < 3)
    return;
  int left = 0,right = 1,sum=1;
  for(;right<n;){
    if(sum == n){
      for(int i=left;i<right;i++)
        cout << i << '+';
      cout << right << '=' << n << endl;
      sum -=left;
      ++left;
    }else if(sum > n){
      sum -= left;
      ++left;
    }else{
      ++right;
      sum += right;
    }
  }
}


int main(){
  int x[] = {2,4,5,2,2,5,5};
  find_continuous_sequence(30);
}
