



#include <iostream> 

using namespace std; 


/* Bubble Sort
 * 两两比较，比较大的泡泡就向上冒
 * 一次扫描确定一个最大的数，总共需要n-1次
 * */

template<class T>
void output(T list[],int size){
  for(int i=0;i<size;i++)
    cout << list[i] << " ";
  cout << endl;
}
template<class T>
void bubbleSort(T list[],int size){
  for(int i=0;i<size-1;i++)   //次数
    for(int j=0;j<size-i-1;j++)   //每一次扫描
      if(list[j] > list[j+1])     
        swap(list[j],list[j+1]);  //两两比较，大者冒泡
  output(list,size);
}

/* 插入排序
 * 不断壮大已排序部分
 * 如何壮大？
 * 从右到左，依次比较，找到合适插入位置
 * 至多插n-1次，从第二个元素开始
 * */ 
template<class T>
void insertSort(T list [],int size){
  int tmp;
  for(int i=1;i<size;i++){
      tmp = list[i];
      int j;
      for(j=i-1;j>=0;j--)
        if(list[j] > tmp)             //小者继续
          list[j+1] = list[j];        //空出位置，以便插入
        else break;                   //找到插入位置
      list[j+1] = tmp;
  }
  output(list,size);
}

int main(){
  const int size = 6;
  int data[]={15,91,48,32,99,10};
  insertSort(data,size);
  return 0;
}
