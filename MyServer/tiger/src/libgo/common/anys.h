

#pragma  once 

#include <sys/types.h>
#include <vector>
#include <mutex>
#include "lock.h"

namespace tiger{
  
  //对象管理
  //先调用Register()将要构造的对象的信息保存在vector中
  //在Anys()构造时，会分配内存，然后根据vector中的信息在该内存进行内存构造
  
  //Anys本质就是一个泛型容器容器
  template <class Group>
    class Anys{
      public:
        using Constructor = void(*)(void*);  //任意类型的函数
        using Destructor = void(*)(void*);  

        template<class T>
          struct DefaultConstructorDestructor{

            //在原始内存上调用placement new 进行构造 ---> 不能直接调用构造函数
            static void Constructor(void* p){
              new (reinterpret_cast<T*>(p)) T();
            }
            //调用析构函数
            static void Destructor(void* p){
              reinterpret_cast<T*>(p)->~T();
            }
          };
      private:
        struct KeyInfo{
          int align;
          int size;
          size_t offset;
        };

        //只有静态成员
        //vector<KeyInfo>  --> 构造的对象的信息
        //mutex,LFLock ---> 用于互斥
        //StoreageLen --> 已经占用的内存长度
        //Size        --> 构造的对象的大小
      inline static std::vector<KeyInfo> & GetKeys(){
        static std::vector<Anys::KeyInfo> obj;
        return obj;
      }
      inline static std::mutex & GetMutex(){
        static std::mutex obj;
        return obj;
      }
      inline static std::size_t & StorageLen(){
        static std::size_t obj = 0;
        return obj;
      }
      inline static std::size_t & Size(){
        static std::size_t obj = 0;
        return obj;
      }
      inline static LFLock & GetLFLock(){
        static LFLock obj;
        return obj;
      }

      public:
       template <typename T>
        static std::size_t Register(){
             return Register<T>(&DefaultConstructorDestructor<T>::Constructor, &DefaultConstructorDestructor<T>::Destructor);
       }
      template <typename T>
        static std::size_t Register(Constructor constructor, Destructor destructor){

          std::unique_lock<std::mutex> lock_guard(GetMutex());   //Register互斥调用

          std::unique_lock<LFLock> lock_guard2(GetLFLock(),std::defer_lock); //若调用register时，Anys对象正在构造 ---> 不能出现这种情况.Register必须在Anys对象构造前调用
        if (!lock_guard2.try_lock())
            throw std::logic_error("Anys::Register mustbe at front of new first instance.");


        KeyInfo info;
        //returns the alignment requirements of the type referrenced
        info.align = std::alignment_of<T>::value;
        //对象大小
        info.size = sizeof(T);
        //对象内存起始点相对于"原点"的偏移
        info.offset = StorageLen();
        info.constructor = constructor;
        info.destructor = destructor;

        GetKeys().push_back(info);
        
        //比如类型A: int a,b,c,d; short int e;
        //alignment_of<A>::value --> 4 : 要求4字节对齐
        //sizeof(A) --> 20 : 从18对齐到20
        //align + info.size - 1 = 4+20-1 = 23 
        //将20字节对齐到4的整数倍地址上 --> 左右偏差不超过3个字节
        //比如当前的20字节的起始地址 0x000101处 --> 调整到0x000104即可
        StorageLen += info.align + info.size-1;

        Size()++;
        return GetKeys().size()-1;  //返回KeyInfo的索引下标
      }
      private:
        char* start,*storage;
        size_t* offsets;

        Anys():start(nullptr),storage(nullptr),offsets(nullptr){
          GetLFLock().try_lock();  //不会释放 --> 构造该对象后就不能再Register了

          //要构造的对象数为0
          if(!Size())
            return;

          //内存申请:假如有n个对象
          //StoreageLen:记录了n个对象最大内存需求
          //另外再申请n+1个size_t大小的空间用于记录
          
          start = (char*)malloc(sizeof(size_t)*(Size()+1)+StorageLen());

          //对象存储空间的起始位置
          storage = start + sizeof(size_t)*(Size()+1);

          //第一个size_t存储对象个数
          *(size_t*)start = Size();

          //后面的每个size_t记录对象相对于storage的偏移
          offsets = (size_t*)(start+sizeof(size_t));



          //对每个对象进行内存对齐，记录其偏移位置，稍后进行构造
          for(size_t i=0;i< Size();i++){

            const auto& KeyInfo = GetKeys()[i];
            //内存对齐前的offset
            size_t offset = KeyInfo.offset;
            std::size_t space = KeyInfo.align + KeyInfo.size - 1;  //同上

            char* base = storage + offset;
            void* ptr;

            //进行内存对齐
            if(!align(KeyInfo.align,KeyInfo.size,ptr,space))
                throw std::logic_error("Anys::get call std::align error");
            
            //内存对齐后的offset
            //比如storage位于0x00101
            //第一个对象的offset为0 ---> base的值为0x00101
            //对象大小为20 ---> 不对齐的情况下，对象的内存起始地址为0x00101
            //若按4字节对齐，则对象的内存起始地址应为0x00102 ---> ptr = 0x00102  ---> 对齐后的offset就为1
            offset += (char*)ptr-base;

            offsets[i] = offset;

            //知道了对象允许的起始地址后，就可以进行对象构造了
          }

            construct();
        }

        ~Anys(){
          destruct();
          if(start){
            free(start);
            start  = offsets = storage = nullptr;
          }
        }
        void reset(){
          destruct();
          construct();
        }


        void construct(){
          if(!Size())
            return;

          for(std::size_t i=0;i<Size();i++){
            const auto& info = GetKeys()[i];
            if(!info.constructor)
              continue;
            //内存地址
            char* p = storage + offsets[i];
            info.constructor(p);
          }
        }
        //对象析构
        void destruct(){
          if(!Size())
            return;

          for(std::size_t i=0;i<Size();i++){
            const auto& info = GetKeys()[i];
            if(!info.destructor)
              continue;
            //内存地址
            char* p = storage + offsets[i];
            info.destructor(p);
         }
       }
    template<class T>
      T& get(size_t idx){  //根据idx获取keyinfo,再根据keyinfo的offset获取对象起始地址
     
        if(idx >= *(size_t*)start)
             throw std::logic_error("Anys::get overflow");
      
        char* p = storage + offsets[idx];
        return *reinterpret_cast<T*>(p);  //从内存指针转为对象指针
      }
    // Copy from g++ std.
    inline void* align(size_t __align, size_t __size, void*& __ptr, size_t& __space) noexcept{
        const auto __intptr = reinterpret_cast<uintptr_t>(__ptr);
        const auto __aligned = (__intptr - 1u + __align) & -__align;
        const auto __diff = __aligned - __intptr;
        if ((__size + __diff) > __space)
            return nullptr;
        else
        {
            __space -= __diff;
            return __ptr = reinterpret_cast<void*>(__aligned);
        }
    }

  };

}

