


#include <iostream>
#include <deque>
#include <queue>
using namespace std; 


//实现在O(1)查找max()的队列
template<class T>
class max_queue{
  public:
    max_queue():idx(0){}

    void push_back(const T& value){
      while(!maximum.empty() && maximum.back().first <= value)
        maximum.pop_back();
      data.push(make_pair(value,idx));
      maximum.push_back(make_pair(value,idx));
      ++idx;
    }
    void pop_front(){
      if(data.empty())
        return;
      if(maximum.front().first == data.front().first)
        maximum.pop_front();
      data.pop();
    }
    
    T max()const {
      if(!maximum.empty()) 
        return maximum.front().first;
      throw;
    }

    void sliding_window_max(T list[],int size,int window_size){
      for(int i=0;i<size;++i){
        push_back(list[i]);
        if(i >= window_size-1){
          if((i-window_size) == maximum.front().second)
            maximum.pop_front();
          cout << max() << endl;
        }
      }
    }
  private:
    queue<pair<T,int>> data;
    deque<pair<T,int>> maximum;
    int idx;
};

int main(){
  max_queue<int> x;
  int a[]={2,3,4,2,6,2,5,1};
  x.sliding_window_max(a,8,3);
}
