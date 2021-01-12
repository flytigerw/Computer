



#include <iostream>
#include <set>
#include <queue>
#include <unistd.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
using namespace std; 


bool isIleagal(char x,char y){
   return ((int)(x-'0')*10+(int)(y-'0')) <= 25;
}
int getTranslation(const char* s,int size){
    if(s == nullptr || size <0)
      return 0;
    int fn_1 = 1,fn_2 = 1,fn = 1;
    for(int i=1;i<size;i++){
        if(isIleagal(s[i-1],s[i]))
         fn = fn_1 + fn_2;
        else 
         fn = fn_1;
        fn_2 = fn_1;
        fn_1 = fn;
   }
    return fn;
}


int get_max_gift(const int* gift,int rows,int cols){
  if(gift == nullptr || rows<0 || cols<0)
    return 0;
  int A[cols];
  for(int i=0;i<rows;i++){
    A[0] += gift[i*cols];
    for(int j=1;j<cols;j++){
        if(i-1 < 0)
          A[j] = A[j-1] + gift[i*cols+j];
        else
          A[j] = max(A[j-1],A[j])+gift[i*cols+j];
        
      }
    }
  return A[cols-1];
}

bool has_the_same(const char*s,int left,int right,char c,int& idx){
  for(int i=left;i<=right;i++){
    if(s[i] == c){
      idx = i;
      return true;
    }
  }
  return false;
}
int get_longest_unques(const char* s,int size){
  if(s == nullptr || size<0)
    return 0;
  int left =0,right = 0;
  int fi_1 = 1,fi=0;
  int idx=0;
  int max_len = 0;
  for(int i=1;i<size;i++,right++){
    if(has_the_same(s,left,right,s[i],idx)){
        fi = max(fi_1,right-left+1);
        left = idx+1;
    }else
      fi = max(fi_1,right-left+1+1);
    if(fi > max_len)
      max_len = fi;
    fi_1 = fi;
  }
  return max_len;
}

int get_the_1500th_ugly_num(){
  int count = 3;
  int m[3] = {2,3,5};
  set<int> s;
  priority_queue<int,vector<int>,greater<int>>q;
  s.insert(2);
  s.insert(3);
  s.insert(5);
  q.push(2);
  q.push(3);
  q.push(5);
  int x;
  int y;
  while(count != 30){
      x = q.top();
      q.pop();
      for(int i=0;i<3;i++){
        y = x*m[i];
        if(s.insert(y).second){
          ++count;
          q.push(y);
          cout << y << endl;
          if(count == 30)
            break;
        }
      }
  }
  return *(--s.end());
}

int get_the_1500th_ugly_num2(){
  int result [1500] = {2,3,4,5};
  int last2 =1,last3=0,last5 =0;
  long M2,M3,M5;
  int count = 4;
  int M =5;
  while(count != 30){
      int i;

      for(i=last2;result[i]*2<=M;i++);
      M2 = 2*result[i];
      last2 = i;
      for(i=last3;result[i]*3<=M;i++);
      M3 = 3*result[i];
      last3 = i;
      for(i=last5;result[i]*5<=M;i++);
      M5 = 5*result[i];
      last5=i;
      
      M = min(min(M2,M3),M5);
      //cout << "M2:" << M2 << "--" << "M3:" << M3 << "--" << "M5:" << M5<< endl;
      cout << "---" << M << endl;
      result[count++] = M;
  }
  return result[30];
}

template<class T>
 void first_not_reapeat(const T* list,int size,T& result){
    if(list == nullptr)
      return;
    unordered_map<T,int> u_map;
    for(int i=0;i<size;i++)
      u_map[list[i]]++;
    for(int i=0;i<size;i++)
      if(u_map[list[i]] == 1){
        result = list[i];
        return;
      }
}

int merge(int* list,int* buf,int first,int mid,int last){
  int i=mid-1,j=last-1,m=last;
  int n=0;
  while(i>=first && j>=mid){
    if(list[i] >list[j]){
      n += (j-mid+1);
      buf[--m] = list[i--];
    }else 
      buf[--m] = list[j--];
  }
  copy_backward(&list[first],&list[i+1],copy_backward(&list[mid],&list[j+1],&buf[m]));
  copy(&buf[first],&buf[last],&list[first]);
  cout << n<< endl;
  sleep(1);
  return n;
}
int partition_and_merge(int* list,int* buf,int first,int last){
 
  if(first == last-1)
    return 0;
  static int count =0;
  int mid=(last+first)/2;
  partition_and_merge(list,buf,first,mid);
  partition_and_merge(list,buf,mid,last);
  count +=merge(list,buf,first,mid,last);
  return count;
}

void reverse_pairs(int* list,int size){
  if(list == nullptr)
    return ;
  int* buf = new int[size];
  cout << partition_and_merge(list, buf,0,size) << endl;
  delete[] buf ;
}

template<class T>
struct node{
  node* next;
  T data;
};

template<class T>
int get_list_lenth(T* list){
  int n = 0;
  for(;list != nullptr;++n,list = list->next);
  return n;
}
template<class T>
node<T>* first_common_node(node<T>* list1,node<T>* list2){
    if(list1 == nullptr || list2 == nullptr)
      return nullptr;
    int len1 = get_list_lenth(list1);
    int len2 = get_list_lenth(list1);
    int dif = abs(len1-len2);
    node<T>* x = (len1 > len2)? list1:list2;
    node<T>* y = (len1 > len2)? list2:list1;
    for(int i=0;i<dif;++i,x=x->next);
    for(;x && y && x!=y;x=x->next,y=y->next);
    if(!x || !y)
      return nullptr;
    return x;
}

int get_numer_same_as_idx(const int* number,int size){
  if(number == nullptr)
    return -1;
  int low = 0;
  int high = size-1;
  int mid = (high+low) >> 1;
  while(low <= high){
    if(number[mid] == mid)
      return mid;
    if(number[mid] < mid)
      low = mid+1;
    else 
      high = mid-1;
    mid = (low+high) >> 1;
  }
  return -1;
}

int main(){
  int a[] ={-3,-1,1,3,5};
  cout << get_numer_same_as_idx(a,5) << endl;
  return 0;
}

