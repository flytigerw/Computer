

#ifndef MAXHEAP
#define MAXHEAP 

#include <sstream>
#include <algorithm>
#include <iterator>
#include "maxPriorityQueue.h"
#include "myExceptions.h"
#include "myArray.h"
#include <memory>
using namespace std;


template<class T>
class maxHeap : public maxPriorityQueue<T>{
  public:
    ~maxHeap(){delete  []heap;}
    //ctors
    maxHeap(int initialCap = 10);
    maxHeap(const maxHeap<T>&);
    //override funcs
    bool empty() const override {return heapSize == 0;}
    int size() const override {return heapSize;}
    const T& top() override {
      if(heapSize == 0)
        throw queueEmpty();
      return heap[0];
    }
    void pop() override;
    void push(const T&) override;
    //other functions
    void initialize(T* , int);
    void output(ostream& out) const;
    T remove(int);
    void deactive(){
      heap = nullptr;
      arrayLength = heapSize = 0;
    }
  private:
    //底层为数组
    T* heap;
    int arrayLength;
    int heapSize;
};

template<class T>
maxHeap<T>::maxHeap(int initialCap){
  if(initialCap < 1){
    ostringstream s;
    s << "Initial capacity = " << initialCap << "must be > 0";
    throw illegalParameterValue(s.str());
  }
  arrayLength = initialCap;
  heapSize = 0;
  heap = new T[arrayLength];
}

template<class T>
maxHeap<T>::maxHeap(const maxHeap<T>& h){
    arrayLength = h.arrayLength;
    heapSize = h.heapSize;
    heap = new T[arrayLength];
    copy(h.heap,h.heap+arrayLength,heap);
}

template<class T>
void maxHeap<T>::pop(){
  if(heapSize == 0)
    throw queueEmpty();
  //delete root element 
  heap[0].~T();
  //remove last element and reserve it 
  --heapSize;
  T lastElement = heap[heapSize-1];
  //find place for lastElement 
  //自上而下，三角比较
  int root = 0;
  int child = 1;
  while(child < heapSize){
    //保证child+1不越界
    //得到较大的子节点
    if(child < heapSize-1 && heap[child] > heap[child+1])
      ++child;
    if(lastElement > heap[child])
      break;
    root = child;
    child = 2*child + 1;
  }
  heap[root]=lastElement;
}

template<class T>
void maxHeap<T>::push(const T& element){
  //空间是否足够
  if(heapSize == arrayLength){
    change1Dlength(heap,arrayLength,arrayLength*2);
    arrayLength *= 2;
  }
  //自下而上进行起泡操作
  int child = heapSize++;
  heap[child] = element;
  int root = (child - 1) / 2 ;
  while(root >= 0 && heap[child] > heap[root]){
      swap(heap[child],heap[root]);
      child = root;
      root = (child - 1) / 2;
    
  }
}

template<class T>
void maxHeap<T>::initialize(T* theHeap,int size){
    //delete old heap 
    delete []heap;
    //update
    heap = theHeap;
    arrayLength = heapSize = size;
    //从最后一个具有子节点的节点开始维护堆序型
    int lastChild = heapSize - 1;
    int lastRoot = (lastChild- 1) / 2;
    int child,root;
    for(int i=lastRoot;i>=0;i--){
        root = i;
        child = root*2+1;
        //自顶向下维护堆序性
        while(child < heapSize){
          if(child < heapSize - 1 && heap[child] > heap[child+1])
            ++child;
          if(heap[root] < heap[child]){
            swap(heap[root],heap[child]);
            root = child ;
            child = root*2 + 1;
          }
          else break; //堆序型完好
        }
    }
}

template<class T>
void maxHeap<T>::output(ostream& out) const{
  copy(heap,heap+heapSize,ostream_iterator<T>(out," "));
}

template<class T>
ostream& operator<<(ostream& out,const maxHeap<T>& h){
  h.output(out);
  return out;
}


template<class T>
T maxHeap<T>::remove(int i) {
  if(i < 0 || i >= heapSize){
    ostringstream s;
    s << "Parameter i = " << i << "must >=0 and < heapSize" << endl;
    throw illegalParameterValue(s.str());
  }
  T element = heap[i];
  heap[i].~T();
  //从i开始向下调整堆序型
  int root = i;
  int child = 2*root+1;
  while(child < heapSize){
    if(child < heapSize-1 && heap[child] < heap[child+1])
      ++child;
    heap[root]= heap[child];
    root = child;
    child = root*2+1;
  }
  return element;
}

//堆排序
template<class T>
void heapSort(T array[],int size){
  maxHeap<T> h;
  h.initialize(array,size);
  //每次提取最大元素
  for(int i=size-1;i>=0;i--){
    array[i] = h.top();
    h.pop();
  }
  h.deactive();
}
#endif
