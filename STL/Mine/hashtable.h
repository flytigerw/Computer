#ifndef HASHTABLE
#define HASHTABLE 

#include "alloc.h"
#include "construct.h"
#include "iterator.h"
#include "algorithm.h"
#include "functional.h"
#include "vector.h"
#include <cstddef>
namespace stl{
    
  template<class T>
    struct hash_node{
      hash_node* next;
      T val;
    };
  
  //类型定义，方便一会儿使用
  template <class Key, class Value, class HashFcn,class ExtractKey, class EqualKey, class Alloc = alloc>
    class hashtable;

  template <class Key, class Value, class HashFcn,class ExtractKey, class EqualKey, class Alloc>
    struct hashtable_iterator;

  template <class Key, class Value, class HashFcn,class ExtractKey, class EqualKey, class Alloc>
    struct hashtable_const_iterator;

  //ExtractKey类似于rb_tree中的KeyOfValue,为仿函数
  //对于hashMap,其元素类型为pair,需要从中提取出key
  //EqualKey:判断键值是否相同的仿函数
  template<class Key,class Value,class HashFcn,class ExtractKey,class EqualKey,class Alloc = alloc>
    struct hashtable_iterator{
      //用类型别名进行简化
      typedef hashtable<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  hashtable;
      typedef hashtable_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  iterator;
      typedef hashtable_const_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  const_iterator;
      typedef hash_node<Value> node;
      //迭代器相关类型
      //只能向前,没有逆向迭代器
      typedef forward_iterator_tag  iterator_category;
      typedef Value                 value_type;
      typedef ptrdiff_t             difference_type;
      typedef Value&                reference;
      typedef Value*                pointer;
      typedef size_t                size_type;

      //数据成员
      //一个节点需要二重定位
      node*  cur;        
      hashtable* ht;    

      //ctors 
      hashtable_iterator(){}
      hashtable_iterator(node* n,hashtable* h) : cur(n),ht(h){
       initialize_buckets(n); 
      }

      reference operator*()const{
        return cur->val;
      }
      pointer operator->()const{
        return &(operator*());
      }
      iterator& operator++(){
        //需要判断cur是否为当前链表的最后一个节点
        //若是，则需要跨桶
        node* old = cur;
        cur = cur->next;
        if(!cur){ //跨bucket
          //根据val值算出bucket的编号
          size_type n = ht->bucket_num(old->val);
          //向前找到第一个非空bucket
          while(!cur && ++n < ht->size())
            cur = ht[n];
        }
        return *this;
      }

      iterator operator++(int){
        iterator tmp = *this;
        operator++();
        return tmp;
      }
      bool operator==(const_iterator& it) const {
        return cur == it.cur;
      }
      bool operator !=(const_iterator& it) const {
        return cur != it.cur;
      }
    };
  //const版本
  template<class Key,class Value,class HashFcn,class ExtractKey,class EqualKey,class Alloc = alloc>
    struct hashtable_const_iterator{
      typedef hashtable<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  hashtable;
      typedef hashtable_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  iterator;
      typedef hashtable_const_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  const_iterator;
      typedef hash_node<Value> node;
      typedef forward_iterator_tag  iterator_category;
      typedef Value                 value_type;
      typedef ptrdiff_t             difference_type;
      typedef const Value&                reference;
      typedef const Value*                pointer;
      typedef size_t                size_type;

      //数据成员 --> 不可通过迭代器修改哈希表中元素
      const node*  cur;        
      const hashtable* ht;    

      //ctors 
      hashtable_const_iterator(){}
      hashtable_const_iterator(node* n,hashtable* h) : cur(n),ht(h){}
      
      reference operator*()const{
        return cur->val;
      }
      pointer operator->()const{
        return &(operator*());
      }
      iterator& operator++(){
        //需要判断cur是否为当前链表的最后一个节点
        //若是，则需要跨桶
        const  node* old = cur;
        cur = cur->next;
        if(!cur){ //跨bucket
          //根据val值算出bucket的编号
          size_type n = ht->bucket_num(old->val);
          //向前找到第一个非空bucket
          while(!cur && ++n < ht->size())
            cur = ht[n];
        }
        return *this;
      }

      iterator operator++(int){
        const iterator tmp = *this;
        operator++();
        return tmp;
      }
      bool operator==(const_iterator& it) const {
        return cur == it.cur;
      }
      bool operator !=(const_iterator& it) const {
        return cur != it.cur;
      }
};
    
    //哈希表打下采用质数
    //当需要扩容时,选择下一个质数作为大小
     static const int primes_num = 28;
     static const unsigned long primes_list[primes_num] ={
        53,         97,           193,         389,       769,
        1543,       3079,         6151,        12289,     24593,
        49157,      98317,        196613,      393241,    786433,
        1572869,    3145739,      6291469,     12582917,  25165843,
        50331653,   100663319,    201326611,   402653189, 805306457, 
        1610612741, 3221225473ul, 4294967291ul};
    
    //给定一个数字,计算得到进挨着的下一质数
inline unsigned long next_prime(unsigned long n){
  const unsigned long* first = primes_list;
  const unsigned long* last= primes_list+primes_num;
  const unsigned long* pos = lower_bound(first,last,n);
  return pos==last ? *(last-1) : *(pos);
} 
  template<class Key,class Value,class HashFcn,class ExtractKey,class EqualKey,class Alloc>
  class hashtable{
      public:
        //类型定义
        typedef Key key_type;
        typedef Value value_type;
        typedef HashFcn hasher;
        typedef EqualKey  key_equal;

        
        typedef size_t            size_type;
        typedef ptrdiff_t         difference_type;
        typedef value_type*       pointer;
        typedef const value_type* const_pointer;
        typedef value_type&       reference;
        typedef const value_type& const_reference;
             
        //数据成员
      private:
        hasher hash;
        key_equal equals;
        ExtractKey  get_key;
        
        typedef hash_node<Value> node;
        typedef simple_alloc<node,Alloc> node_allocator;
        
        vector<node*,Alloc>  buckets;
        size_type  elements_num;

      public:
        //友元声明
        typedef hashtable_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  iterator;
        typedef hashtable_const_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>  const_iterator;
        friend struct hashtable_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc>;
        friend struct hashtable_const_iterator<Key,Value,HashFcn,ExtractKey,EqualKey,Alloc> ;
        friend bool operator==(const hashtable&,const hashtable&);
      public:
        //ctors
        hashtable(size_type n,const HashFcn& hf,const EqualKey& eql,const ExtractKey& ext) : hash(hf),equals(eql),get_key(ext),elements_num(0){
          initialize_buckets(n);
        }
        hashtable(const hashtable& h):hashtable(h.hash),equals(h.equals),get_key(h.get_key),elements_num(0){
          copy_from(h);
        }
        hashtable& operator=(const hashtable& h){
          if(&h != this){
            //销毁源数据
            clear();
            hash = h.hash;
            equals = h.equals;
            get_key = h.get_key;
            copy_from(h);
          }
          return *this;
        }

        ~hashtable(){clear();}
        //对外接口
      public:
        size_type size() const {return elements_num;}
        size_type max_size() const {return size_type(-1);}
        bool empty() const {return elements_num == 0;}
        
        iterator begin(){
              //第一个非空的bucket的第一个元素处
              for(size_type n =0;n<buckets.size();++n)
                if(buckets[n])
                  return iterator(buckets[n],this);
              return end();
        }

        iterator end(){
          return iterator(nullptr,this);
        }

        const_iterator cbegin() const{
              for(size_type n =0;n<buckets.size();++n)
                if(buckets[n])
                  return const_iterator(buckets[n],this);
              return cend();
        }
        const_iterator cend() const{
          return const_iterator(0,this);
        }

        //buckets数量
        size_type bucket_count() const{
          return buckets.size();
        }
        //最大的buckets数量
        size_type max_bucket_count() const{
          return primes_list[primes_num-1];
        }

        //指定bucket里元素的数量
        size_type elements_num_in_bucket(size_type n){
          size_type x = 0;
          for(auto cur=buckets[n];cur;cur=cur->next)
            ++x;
          return x;
        }
        
        //swap ----> 底层为vector ---> 调用其vector
        void swap(hashtable& ht){
          std::swap(hash,ht.hash);
          std::swap(equals,ht.equals);
          std::swap(get_key,ht.get_key);
          buckets.swap(ht.buckets);
          std::swap(elements_num,ht.elements_num);
        }
          

        //插入系列
        std::pair<iterator,bool> insert_unique(const value_type& v){
          //若elements_num + 1 > buckets.size() --> 负载系数较高
          resize(elements_num+1);
          return insert_unique_noresize(v);
        }
        iterator insert_equal(const value_type& v){
           resize(elements_num + 1);
          return insert_equal_noresize(v);
        }

        std::pair<iterator,bool>  insert_unique_noresize(const value_type& v){
          //需要先找到插入位置
          //桶号
          const size_type n = bucket_num(v);
          for(auto cur = buckets[n]; cur != nullptr ; cur = cur->next)
            //比较key
            if(equals(get_key(cur->val),get_key(v)))
              return std::make_pair(iterator(cur,this),false);  //节点存在
          
          //生成新节点,插在桶的前端
          node* x = new_node(v);
          x->next = buckets[n];
          buckets[n] = x;
          ++elements_num;
          return std::make_pair(iterator(x,this),true);
        }

        
        iterator  insert_equal_noresize(const value_type& v){
          //类似insert_unique_noresize,只不过键值可以重复
          const size_type n = bucket_num(v);
          node* x = new_node(v);
          ++elements_num;
          for(auto cur = buckets[n]; cur != nullptr ; cur = cur->next)
            if(equals(get_key(cur->val),get_key(v))){
              //就插在cur后面
              x->next = cur->next;
              cur->next = x;
              return iterator(x,this);
            }
          x->next = buckets[n];
          buckets[n] = x;
          return iterator(x,this);
        }
        
        //insert迭代器范围内的元素
        template<class InputIterator>
        void insert_unique(InputIterator first,InputIterator last){
              insert_unique(first,last,iterator_category(first));
        }
        template <class InputIterator>
        void insert_unique(InputIterator f, InputIterator l, input_iterator_tag){
          for ( ; f != l; ++f)
             insert_unique(*f);
        }

        template <class ForwardIterator>
        //ForwardIterator可以计算迭代器之间的距离
        void insert_unique(ForwardIterator f, ForwardIterator l, forward_iterator_tag){
            size_type n = 0;
            distance(f, l, n);
            //resize的个数一次搞定
            resize(elements_num+ n);
            //以n作为循环条件更快
            for ( ; n > 0; --n, ++f)
             insert_unique_noresize(*f);
        }

      //Equal版本
        template<class InputIterator>
        void insert_equal(InputIterator first,InputIterator last){
              insert_equal(first,last,iterator_category(first));
        }
        template <class InputIterator>
        void insert_equal(InputIterator f, InputIterator l, input_iterator_tag){
          for ( ; f != l; ++f)
             insert_equal(*f);
        }

        template <class ForwardIterator>
        void insert_equal(ForwardIterator f, ForwardIterator l, forward_iterator_tag){
            size_type n = 0;
            distance(f, l, n);
            resize(elements_num+ n);
            for ( ; n > 0; --n, ++f)
             insert_equal_noresize(*f);
        }
      


        //查找系列
        iterator find(const key_type& key){
            //先找桶，再找节点
            size_type n = bkt_num_key(key);
            node* cur;
            for(cur = buckets[n]; cur && !equals(get_key(cur->val),key); cur = cur->next){}
            return iterator(cur,this);
        }
        const_iterator find(const key_type& key) const {
            //先找桶，再找节点
            size_type n = bkt_num_key(key);
            node* cur;
            for(cur = buckets[n]; cur && !equals(get_key(cur->val),key); cur = cur->next){}
            return const_iterator(cur,this);
        }

        //计算有多少个相同的节点 --> k值相同
        size_type count(const key_type& key) const{
            size_type n = bkt_num_key(key);
            size_type c = 0;
            node* cur;
            for(cur = buckets[n]; cur; cur = cur->next){
              if(equals(get_key(cur->val),key))
                 ++c;
            }
            return c;
        }
        
        //找不到就插入
        reference find_or_insert(const value_type& v){
            size_type n = bucket_num(v);
            node* cur;
            for(cur = buckets[n]; cur; cur = cur->next){
              if(equals(get_key(cur->val),get_key(v)))
                return cur->val;
            }

            //没找到，插在bucket前端
            cur = new_node(v);
            cur->next = buckets[n];
            buckets[n]->next = cur;
            ++elements_num;
            return cur->val;
        }

       //元素k在表中的范围
        std::pair<iterator,iterator>
          equal_range(const key_type& k){
            const size_type n = bkt_num_key(k);
            node* cur;
            node* tmp;
            //定位到桶，桶中寻找
            for(cur = buckets[n];cur;cur=cur->next){
              if(equals(get_key(cur->val),k)){
                
                for(tmp = cur->next;tmp;tmp=tmp->next)
                  if(!equals(get_key(tmp->val),k))
                    return std::make_pair(iterator(cur,this),iterator(tmp,this));

                //bucket的尾端的key与k一致 ---> 寻找下一个非空bucket,用其首元素作为结束
                for(auto m = n+1;m<buckets.size();++m)
                  if(buckets[m])
                    return std::make_pair(iterator(cur,this),iterator(buckets[m],this));
                
                return std::make_pair(iterator(cur,this),end());
              }
            }
          return std::make_pair(end(),end());
          }
        //const版本
        std::pair<const_iterator,const_iterator>
          equal_range(const key_type& k) const {
            const size_type n = bkt_num_key(k);
            node* cur;
            node* tmp;
            //定位到桶，桶中寻找
            for(cur = buckets[n];cur;cur=cur->next){
              if(equals(get_key(cur->val),k)){
                for(tmp = cur->next;tmp;tmp=tmp->next)
                  if(!equals(get_key(tmp->val),k))
                    return std::make_pair(const_iterator(cur,this),const_iterator(tmp,this));
                //bucket的尾端的key与k一致 ---> 寻找下一个非空bucket,用其首元素作为结束
                for(auto m = n+1;m<buckets.size();++m)
                  if(buckets[m])
                    return std::make_pair(const_iterator(cur,this),const_iterator(buckets[m],this));
                
                return std::make_pair(const_iterator(cur,this),cend());
              }
            }
          return std::make_pair(cend(),cend());
          }


        //删除系列
        //返回被删除的个数 --> 重复的关键字
        size_type erase(const key_type& k){
            //先要找到桶，然后在遍历链表执行删除
            const size_type n = bkt_num_key(k);
            size_type count = 0;
            node* cur,*next;
            auto first = buckets[n];
            if(first)
              cur = buckets[n];
              next = cur->next;
              while(next){
                if(equals(get_key(next->val),k)){
                  cur->next = next->next;
                  delete_node(next);
                  ++count;
                  --elements_num;
                }else{
                  cur = next;
                  next = next->next;
                }
              }
            if(equals(get_key(first->val,k))){
                buckets[n] = first->next;
                delete_node(first);
                ++count;
                --elements_num;
            }
        }

        //通过迭代器删除
        void erase(const iterator& it){
          //需要找到前一个位置才能在链表中进行删除
          if(auto p = it.cur){
            const size_type n = bucket_num(p->val);
            node* cur = buckets[n];
            if(cur == p){
              buckets[n] = cur->next;
              delete_node(cur);
              --elements_num;
            }else{
              node* next = cur->next;
              while(next){
                if(next == p){
                  cur->next = next->next;
                  delete_node(next);
                  --elements_num;
                  break;
                }else{
                  cur = next;
                  next = next->next;
                }
              }
            }
          }
        }
        
        //删除范围内的元素 ---> 有可能跨桶
        void erase(iterator first,iterator last){
              size_type f_bucket = first ? bucket_num(first.cur->val) : buckets.size();
              size_type l_bucket = last ? bucket_num(last.cur->val) : buckets.size();
              if(first.cur == last.cur)
                return;
              else if(f_bucket == l_bucket)
                erase_bucket(f_bucket,first.cur,last.cur);
              else{
                //跨buckets ---> 分三段删除
                erase_bucket(f_bucket,first.cur,nullptr);
                for(auto i=f_bucket+1;i<l_bucket;i++)
                  erase_bucket(i,nullptr);
                if(l_bucket != buckets.size())
                  erase_bucket(l_bucket,last.cur);
              }
        }
        
        //const版本
        void erase(const const_iterator& it){
          erase(iterator(
                const_cast<node*>(it.cur),
                const_cast<hashtable*>(it.ht)
                ));
        }

        void erase(const_iterator first,const_iterator last){

         erase(iterator(const_cast<node*>(first.cur),
                 const_cast<hashtable*>(first.ht)),
               iterator(const_cast<node*>(last.cur),
                 const_cast<hashtable*>(last.ht)));
        }

        void clear();
      //调整buckets数量 ---> 对原来的元素需要重新散列
        void resize(size_type);
        //私有辅助函数
      private:
        //下一次桶的个数
        size_type next_size(size_type n) const {
          return  next_prime(n);
        }

        //对n个buckets进行初始化 --> 预留n个buckets的空间, 构造为nullptr
        void initialize_buckets(size_type n){
              const size_type size = next_size(n);
              buckets.reserve(size);
              buckets.insert(end(),size,nullptr);
              elements_num = 0;
        }

        //根据当前元素的value值计算出其位于哪一个buckets中 ---> 需要得到键值 ---> 再对键值散列
        size_t bucket_num(const value_type& v) const{
          return bkt_num_key(get_key(v));
        }
        //根据key值得到bucket_num
        size_t bkt_num_key(const key_type& k) const{
          return hash(k) % buckets.size();
        }
        //含有提示值hint
        size_t bucket_num(const value_type& v,size_type hint) const{
          return bkt_num_key(get_key(v),hint);
        }
        size_t bkt_num_key(const key_type& k,size_type hint) const{
          return hash(k) % hint;
        }       

        //新建一个节点:分配内存，构造内存
        node* new_node(const value_type& v){
          node* n = node_allocator::allocate();
          n->next = nullptr;
          try{
            construct(&n->val,v);
            return n;
          }catch(...){
            node_allocator::deallocate(n);
            throw;
          }
        }


        //销毁一个节点
        void delete_node(node* n){
          destory(&n->val);
          node_allocator::deallocate(n);
        }


        //将另一个hashtable的所有元素拷贝过来
        void copy_from(const hashtable& h);



        //删除一个桶内的范围内元素
        void erase_bucket(const size_type n,node* first,node* last){
            node* cur = buckets[n];
            if(last == last)
              erase_bucket(n,last);
            else{
              node* next = cur->next;
              //先找到first
              for(;next != first;cur=next,next=cur->next);
              while(next){
                cur->next = next->next;
                delete_node(next);
                next = cur->next;
                --elements_num;
              }
            }
        }
        //删除某个bucket的[beg,last)范围的元素
        void erase_bucket(const size_type n,node* last){
          node* cur = buckets[n];
          node* next;;
          while(cur != last){
              next = cur->next;
              delete_node(cur);
              cur = next;
              buckets[n] = cur;
              --elements_num;
          }
        }
  };

template<class K,class V,class HF,class Ex,class Eq,class A>
bool operator==(const hashtable<K,V,HF,Ex,Eq,A>& ht1,const hashtable<K,V,HF,Ex,Eq,A>& ht2){
        if(ht1.buckets.size() != ht2.buckets.size())
          return false;
        typedef typename hashtable<K,V,HF,Ex,Eq,A>::node node;
        //逐个比较
        node* cur1;
        node* cur2;
        for(int i=0;i<ht1.buckets.size();++i){
            cur1 = ht1.buckets[i];
            cur2 = ht2.buckets[i];
            for(;cur1 && cur2 && cur1->val == cur2->val;cur1 = cur1->next,cur2=cur2->next){}
            if(cur1 || cur2)
              return false;
        }

        return true;
}

template<class K,class V,class HF,class Ex,class Eq,class A>
void hashtable<K,V,HF,Ex,Eq,A>::resize(size_type hint){
      const size_type old_size = buckets.size();     

      if(hint > old_size){ //向上调整容量 
          const size_type new_size = next_size(hint);

          //新建buckets,对原来的每个元素重新散列
          vector<node*,A> tmp(new_size,nullptr);
          try{
            for(size_type i=0;i<old_size;i++){
                node* x = buckets[i];
                while(x){
                //元素重新散列
                  size_type new_bucket_num = bucket_num(x->val,new_size);
                  buckets[i] = x->next;
                  //插在新的bucket的前端
                  x->next = tmp[new_bucket_num];
                  tmp[new_bucket_num] = x;
                  x = buckets[i];
                }
            }
            buckets.swap(tmp);
          }catch(...){
                //已经重新散列的全部删除
            for(size_type i=0;i<tmp.size();++i){
              while(tmp[i]){
                node* x = buckets[x]->next;
                delete_node(tmp[i]);
                tmp[i] = x;
              }
            }
            throw ;
          }

      }
}

template<class K,class V,class HF,class Ex,class Eq,class A>
void hashtable<K,V,HF,Ex,Eq,A>::clear(){
   //逐个node的销毁
  node* cur,next;
  for(size_type i =0;i<buckets.size();i++){
    cur = buckets[i];
    while(cur){
      next = cur;
      delete_node(cur);
      cur = next;
    }
    buckets[i] =nullptr;
  }
  elements_num = 0;
}

template<class K,class V,class HF,class Ex,class Eq,class A>
void hashtable<K,V,HF,Ex,Eq,A>::copy_from(const hashtable& ht){
    //将表1的数据拷贝到表2
    
    //预留空间
    buckets.reserve(ht.buckets.size()); //分配了指针内存
    buckets.insert(buckets.end(), ht.buckets.size(), (node*) 0);
    //逐个拷贝
    node* cur;
    node* x;
    try{
      for(size_type i=0;i<ht.buckets.size();i++){
        if((cur = ht.buckets[i])){
            x = new_node(cur->val);
            buckets[i] = x;
            for(cur = cur->next;cur != nullptr;cur=cur->next){
              x->next = new_node(cur->val);
              x = x->next;
            }
        }
      }
      elements_num = ht.elements_num;
    }catch(...){
      clear();
      throw ;
    }
}

}
#endif
