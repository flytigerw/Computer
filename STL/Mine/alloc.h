#ifndef  ALLOC 
#define  ALLOC

#include <unistd.h>
#include <sys/stat.h>
#include <memory>
#include <string.h>

namespace stl{
  //内存分配的对外接口
  template<class T,class Alloc>
    class simple_alloc{
      public:
        static T* allocate(size_t n){
          return (n==0)? nullptr : (T*)Alloc::allocate(n*sizeof(T));
        }
        static T* allocate(void){
          return (T*)Alloc::allocate(sizeof(T));
        }
        static void deallocate(T*p, size_t n){
          if(n != 0)
            Alloc::deallocate(p,n*sizeof(T));
        }
        static void deallocate(T* p){
          Alloc::deallocate(p,sizeof(T));
        }
    };
  
  //内存错误处理
#include <iostream>
#define THORW_BAD_ALLOC std::cerr << "out of memory" << std::endl; exit(1);

  //第一级配置器，当需求量>128kb时，采用malloc
    class malloc_alloc{
      public: 
        using malloc_alloc_handler =void(*)();

        static void* allocate(size_t n){
          void* result = malloc(n);
          if(result == nullptr)
            result = dispose_bad_malloc(n);
          return result;
        }
        
        //size_t 与外层接口保持一致
        static void deallocate(void* p,size_t){
          free(p);
        }
        
        static void* reallocate(void* p,size_t,size_t new_size){
          void * result = realloc(p,new_size);
          if(result == nullptr)
            result = dispose_bad_realloc(p,new_size);
          return result;
        }

        static  malloc_alloc_handler set_malloc_handler(malloc_alloc_handler new_handler){
            malloc_alloc_handler old_handler = user_defined_handler; 
            user_defined_handler = new_handler;
            return old_handler;
        }
       private:
         //处理malloc时的内存不足问题
         static void* dispose_bad_malloc(size_t);
         //处理器realloc时的内存不足问题
         static void* dispose_bad_realloc(void*,size_t);        
         //使用者定义内存不足时的处理手段
         static malloc_alloc_handler user_defined_handler;
    };

    malloc_alloc::malloc_alloc_handler malloc_alloc::user_defined_handler = nullptr;
    void* malloc_alloc::dispose_bad_malloc(size_t n){
          void* result;
          for(;;){
            if(user_defined_handler == nullptr)
              THORW_BAD_ALLOC;
            //调用处理程序 ---->  也许是释放一些内存
            user_defined_handler();
            //重新申请
            result = malloc(n);
            if(result != nullptr)
              return result;
          }
    }

    void* malloc_alloc::dispose_bad_realloc(void* p,size_t n){
      void* result;
      for(;;){
         if(user_defined_handler == nullptr)
           THORW_BAD_ALLOC;
         //调用处理程序 ---->  也许是释放一些内存
         user_defined_handler();
         //重新申请
         result = realloc(p,n);
         if(result != nullptr)
            return result;
      }
    }

  //通过macro来来选择使用哪一级配置器
#ifdef USE_MALLOC 
    typedef malloc_alloc alloc;
    typedef malloc_alloc single_clien_alloc;
#else 

      class memory_pool_alloc{
        private:
         enum{
           ALIGN = 8,
           MAX_BYTES = 128,
           //自由链表数量
           FREESTLISTS_NUM = MAX_BYTES / ALIGN
         };
         
         //在自由链表中，花费一根指针指向下一块内存
         //但分配给用户以后，呈现为char [1] ---> 在用户眼中没有这一个根指针的开销
         union obj{
            union obj* next;
            char client_data[1];
         };
         
         //自由链表池
         static obj* volatile free_lists[FREESTLISTS_NUM];
         //内存池
         static char* start_free;
         static char* end_free;
         //累计申请量
         static size_t total_got;

         //将一个数上调至8的倍数
         static size_t round_up(size_t n){
              //采用位运算
              //先上调，再截取低位
              return (n+ALIGN-1) & ~(ALIGN-1);
         }
         //获取对应的链表在链表池中的下标:31 --> 38/8-1
         static size_t get_index(size_t n){
              //上调，取商
              return (n+ALIGN-1) / ALIGN - 1;
         }
         //返回一块内存
         static void* refill(size_t n);
         //检查pool余量，考虑是否重新申请内存
         static char* chunk_alloc(size_t size,int &nodes);
      public:
         static void* allocate(size_t n){
           //需求过大，采用一级配置器
           if(n > (size_t) MAX_BYTES)
             return stl::malloc_alloc::allocate(n);
           //找到对应的自由链表,得到第一块
           //若为空,则调用refill
           //若不为空，则将第一块分配出去
           obj* volatile result;
           obj* volatile * list= free_lists+get_index(n);
           result = *list;
           if(result == nullptr){
             void* r= refill(round_up(n));
             return r;
           }
           *list = result->next;
           return result;
         }
         
         static void deallocate(void* p,size_t n){
           if(n>(size_t)MAX_BYTES){
             stl::malloc_alloc::deallocate(p,n);
             return ;
           }
           //将该块内存归还给相应的自由链表
           obj* volatile * free_list = free_lists+get_index(n);
           obj* q = (obj*)p;
           q->next = *free_list;
           *free_list = q;
         }

         static void* reallocate(void*p,size_t old_size,size_t new_size){
            if(old_size > (size_t)MAX_BYTES && new_size>(size_t)MAX_BYTES)
              return realloc(p,new_size);
            if(round_up(old_size) == round_up(new_size))
              return p;
            void* result = allocate(new_size);
            size_t copy_size = (new_size>old_size) ? old_size : new_size;
            memcpy(result,p,copy_size);
            deallocate(p,old_size);
              return result;
         }
};


         memory_pool_alloc::obj* volatile memory_pool_alloc::free_lists[FREESTLISTS_NUM] = {nullptr};
         //内存池
         char* memory_pool_alloc::start_free = nullptr;
         char* memory_pool_alloc::end_free = nullptr;
         size_t memory_pool_alloc::total_got = 0;
//调用时，自由链表为空。
//重新填充
void* memory_pool_alloc::refill(size_t size){
  int n = 20;
  //可能一次得到了多块，但只需要分配一块出去
  char* chunks = chunk_alloc(size,n);
  if(n == 1)
    return chunks;
  
  //返回的是大块内存的首地址 ---> 对其进行按块切割
  obj* volatile* list = free_lists+get_index(size);
  //第一块地址
  obj* result = (obj*)chunks;  
  obj* current_obj,*next_obj;
  //将其交付给相应的自由链表
  *list = next_obj = (obj*)(chunks + size);
  //进行切割
  for(int i=1;;i++){
    current_obj = next_obj;
    next_obj = (obj*)((char*)next_obj+size);
    if(i == n-1){
      current_obj->next = nullptr;
      break;
    }else 
      current_obj->next = next_obj;
  }
  //将第一块的地址给出去
  return result;
}

//理想条件下，能够一次分配n = 20块
char* memory_pool_alloc::chunk_alloc(size_t size,int& n){
      size_t total_need = size*n;
      //内存池的内存余量
      size_t bytes_left= end_free - start_free;
      char* result;
      //剩余量足够
      if(bytes_left >= total_need){
         result = start_free;
         //更新内存池
         start_free += total_need;
         return result;
      }else if(bytes_left >= size){
         //至少能分配一块
         //改变块数
         n = bytes_left / size;
         total_need = size*n;
         result = start_free;
         start_free += total_need;
         return result;
      }else{
        //一块也不够
        //进行碎片处理
        if(bytes_left > 0){
          //将内存碎片拨给对应的自由链表
          //比如申请104byte,bytes_left = 80byte 
          //将其拨给第9号链表,插到链表头
          obj* volatile* list = free_lists + get_index(bytes_left); 
          ((obj*)start_free) -> next = *list;
          *list = (obj*)start_free;
        }
        //没有。重新申请
        //申请量
        size_t bytes_to_get = 2*total_need + round_up(total_got >> 4);
        start_free = (char*) malloc(bytes_to_get);

        if(start_free == nullptr){  //申请失败 --> 内存耗尽
          //向左search自由链表，若其含有剩余内存，则拿来使用
          obj* volatile* list;
          obj* p;
          for(int i=size;i<=MAX_BYTES;i+=ALIGN){
            list = free_lists+get_index(i);
            p = *list;
            //含有余量 --> 将第一块分配给内存池 ---> 剩余的依旧有该链表掌控
            if(p != nullptr){
              *list= p->next;
              start_free = (char*)p;
              end_free  = start_free + i;
              //下一次分配肯定能通过 ---> 内存余量至少有一块大小
              return (chunk_alloc(size,n));
            }
          }
          //山穷水尽 ---> 采用malloc_alloc --> 其有分配失败的处理
          end_free = nullptr;
          start_free = (char*) stl::malloc_alloc::allocate(bytes_to_get);
        }
          //失败处理后，若程序到这，说明分配成功
          total_got += bytes_to_get;
          end_free = start_free + bytes_to_get;
          return (chunk_alloc(size,n));
      }
}
typedef memory_pool_alloc alloc;
#endif
}
#endif
