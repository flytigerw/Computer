


#ifndef HASHMULTIMAP
#define HASHMULTIMAP 



#include "functional.h"
#include "hashfunction.h"
#include "hashtable.h"
namespace stl{

  //Key和Value是不同,组成pair,传给底层node --> 底层node的val为pair ---> 使用时需要提取出key ---> 采用仿函数select_first
  template<class Key,class Value,class HashFcn = hash<Key>,class EqualKey = equal_to<Key>,class Alloc = alloc>
    class hash_multimap{
      private:
        //不能修改hashtable的Key,但可以修改Key对应的Value
        //迭代器是将二者绑在一起的，不能从迭代器下手，只能从更高层的pair下手
        typedef hashtable<Key,std::pair<const Key,Value>,HashFcn,select_first<std::pair<const Key,Value>>,EqualKey,Alloc>   hashtable;
        hashtable ht;

      public:
        //对外类型定义
        typedef typename hashtable::key_type key_type;
        typedef typename hashtable::value_type value_type;      
        typedef Value    data_type;
        typedef Value    mapped_type;
        typedef typename hashtable::hasher hasher;
        typedef typename hashtable::key_equal key_equal;

        //迭代器相关 ---> 不能修改hashtable中元素值 --> 其元素有固定排序
        typedef typename hashtable::difference_type difference_type;
        typedef typename hashtable::pointer pointer;
        typedef typename hashtable::const_pointer const_pointer;
        typedef typename hashtable::_reference reference;
        typedef typename hashtable::const_reference const_reference;
        typedef typename hashtable::iterator const_iterator;
        typedef typename hashtable::iterator iterator;
        typedef typename hashtable::size_type size_type;;

      public :
        //默认有100+个buckets
        //haser() ---> hash<key>() 临时对象
        //key_equal --> euqal_to<key>() 临时对象
        hash_multimap():hashtable(100,hasher(),key_equal()){}

        template<class InputIterator>
          hash_multimap(InputIterator first,InputIterator last,size_type n) : hashtable(n,hasher(),key_equal()){
            ht.insert_equal(first,last);
          }
        template<class InputIterator>
          hash_multimap(InputIterator first,InputIterator last) : hashtable(100,hasher(),key_equal()){
            ht.insert_equal(first,last);
          }
        //指定hash
        template<class InputIterator>
          hash_multimap(InputIterator first,InputIterator last,size_type n,const hasher& h) : hashtable(n,h,key_equal()){
            ht.insert_equal(first,last);
          }

        //对外接口
      public:
        size_type size() const{
          return ht.size();
        }
        size_type max_size() const{
          return ht.max_size();
        }
        bool empty() const {
          return ht.empty();
        }
        void swap(hash_multimap& x){
          ht.swap(x.ht);
        }
        
        iterator begin() {
          return ht.begin();
        }
        iterator end() {
          return ht.end();
        }
        const_iterator begin() const{
          return ht.cbegin();
        }
        const_iterator end() const{
          return ht.cend();
        }

        template<class InputIterator>
          void insert(InputIterator first,InputIterator last){
            ht.insert_equal(first,last);
          }

        std::pair<iterator,bool> insert_noresize(const value_type& v){
          auto p = ht.insert_equal_noresize(v);
          return p;
        }

        iterator find(const key_type& k) {
          return ht.find(k);
        }
        const_iterator find(const key_type& k) const{
          return ht.find(k);
        }

        //支持下标操作
        //没找到就插入
        Value& operator[](const key_type& k){
          return ht.find_or_insert(value_type(k,Value())).second;
        }

        size_type count(const key_type& key) const{
          return ht.count(key);
        }
        std::pair<iterator, iterator> equal_range(const key_type& key) {
          return ht.equal_range(key);
        }
        std::pair<const_iterator,const_iterator> equal_range(const key_type& key) const{
          return ht.equal_range(key);
        }

        size_type erase(const key_type& key) {
          return ht.erase(key);
        }
        void erase(iterator it) { ht.erase(it); }
        void erase(iterator f, iterator l) { ht.erase(f, l); }
        void clear() { ht.clear(); }

      public:

         void resize(size_type hint) { ht.resize(hint); }
         size_type bucket_count() const { return ht.bucket_count(); }
         size_type max_bucket_count() const { return ht.max_bucket_count(); }
         size_type elems_in_bucket(size_type n) const{          return ht.elems_in_bucket(n); }
    };

template <class Value, class HashFcn, class EqualKey, class Alloc>
inline bool operator==(const hash_multimap<Value, HashFcn, EqualKey, Alloc>& hs1,
                       const hash_multimap<Value, HashFcn, EqualKey, Alloc>& hs2)
{
  return hs1.ht == hs2.ht;
}
template <class Val, class HashFcn, class EqualKey, class Alloc>
inline void swap(hash_multimap<Val, HashFcn, EqualKey, Alloc>& hs1,
                 hash_multimap<Val, HashFcn, EqualKey, Alloc>& hs2) {
  hs1.swap(hs2);
}
}
#endif
