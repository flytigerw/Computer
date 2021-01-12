
#include <iostream>
using namespace std;

class screen{
  public:
    screen(int x) : data(x){}
    static void* operator new(size_t size){
      screen* p;
      if(!free_store){
        size_t chunk = screen_chunk* size;
        free_store = p =reinterpret_cast<screen*>(new char[chunk]);
        for(;p != &free_store[screen_chunk-1]; p++)
          p->next = p+1;
        p->next = nullptr;
      }
      p = free_store;
      free_store = free_store->next;
      return p;
    }
    static void  operator delete(void* p,size_t ){
      static_cast<screen*>(p) -> next = free_store;
      free_store = static_cast<screen*>(p);
    }
  private:
    screen* next;
    static screen* free_store;
    static constexpr int screen_chunk = 24;
    int data;
};

screen* screen::free_store = nullptr;

int main(){
  cout << sizeof(screen) << endl;
  size_t const n = 10;
  screen* p[n];
  for(size_t i=0;i<n;i++)
    p[i] = new screen(i);

  for(size_t i=0;i<n;i++)
    cout << p[i] << endl;
  for(size_t i=0;i<n;i++)
    delete p[i];
}
