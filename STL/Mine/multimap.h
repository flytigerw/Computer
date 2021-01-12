


#ifndef MULTIMAP
#define MULTIMAP 


//类似multimap，只不过关键字可以重复
#include "functional.h"
#include "rbTree.h"
namespace stl{
  
  template<class Key,class T,class Compare = less<Key>,class Alloc = alloc>
    class multimap{
      public:
        //类型定义
        typedef Key key_type;
        typedef T   data_type;
        typedef T   mapped_type;
        //关键字为const --> 若为非const,用户可修改关键字值 ---> 但红黑树里关键字的顺序应该是固定的
        typedef std::pair<const Key,T>value_type;  //key和value是一致的
        typedef Compare key_compare;
        

      private:
        typedef rb_tree<key_type,select_first<value_type>,value_type,key_compare,Alloc> rb_tree_type;
        rb_tree_type t;
        
        //迭代器相关-->rb_tree的迭代器
        //不能通过迭代器修改value 
      public:
        typedef typename rb_tree_type::pointer pointer;
        typedef typename rb_tree_type::const_pointer const_pointer;
        typedef typename rb_tree_type::reference reference;
        typedef typename rb_tree_type::const_reference const_reference;
        typedef typename rb_tree_type::iterator iterator;
        typedef typename rb_tree_type::const_iterator const_iterator;
        typedef typename rb_tree_type::reverse_iterator reverse_iterator;
        typedef typename rb_tree_type::const_reverse_iterator const_reverse_iterator;
        typedef typename rb_tree_type::size_type size_type;
        typedef typename rb_tree_type::difference_type difference_type;
        
    //默认构造函数会function object的临时对象
    multimap():t(Compare()){}
    //显示地执行比较器
    explicit multimap(const Compare& comp) : t(comp){}
    //用迭代器范围进行初始化
    template<class InputIterator>
      multimap(InputIterator first,InputIterator last):t(Compare()){
        t.insert_equal(first,last);
      }

    template<class InputIterator>
      multimap(InputIterator first,InputIterator last,const Compare& comp):t(comp){
        t.insert_equal(first,last);
      }
    
    //会调用底层rbTree的拷贝构造函数
    multimap(const multimap<Key,Compare,Alloc>& x) : t(x.t){ }
    multimap<Key,Compare,Alloc>& operator=(const multimap<Key,Compare,Alloc>& x){
      t  = x.t;
      return *this;
    }

    //对外接口
    key_compare key_comp() const {return t.key_comp();}
    iterator begin() const { return t.begin(); }
    iterator end() const { return t.end(); }
    reverse_iterator rbegin() const { return t.rbegin(); } 
    reverse_iterator rend() const { return t.rend(); }
    bool empty() const { return t.empty(); }
    size_type size() const { return t.size(); }
    size_type max_size() const { return t.max_size(); }
    //swap底层rb_tree的相关指向
    void swap(multimap<Key, Compare, Alloc>& x) { t.swap(x.t); }

    //支持[]操作 ---> 以关键字作为下标，返回对应value的引用.
    //执行的操作为 insert(value_type(k,T())) --> 返回pair<iterator,bool> --> 对迭代器解引用得到底层的pair<const key,value> ---> second就为value
    //可见:若k不存在,则会成功插入新的元素对<k,T()>
    //若k存在,则会返回元素对<k,value> 
    T& operator[](const value_type& k) {
      return  ((*(insert(value_type(k,T())))).first).second;
    }
    
    

    //插入和删除
    //插入元素,返回pair,可以通过bool字段来表明该元素是否已经存在
    std::pair<iterator,bool>
      insert(const value_type& x){
        return t.insert_equal(x);
      }
    
    //在某一位置后插入元素,返回指向新元素的迭代器
    iterator insert(iterator position, const value_type& x) {
      return t.insert_equal(position, x);
  }
    //插入范围
    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
      t.insert_equal(first, last);
     }
    //根据位置删除
   void erase(iterator position) { 
      t.erase(position); 
    }
    //根据元素删除
    size_type erase(const key_type& x) { 
      return t.erase(x); 
     }
    //删除范围
    void erase(iterator first, iterator last) { 
      t.erase(first,last); 
    }
    //查找类操作
    //根据元素查找,返回迭代器
    iterator find(const key_type& k) const{
      return t.find(k);
    }
    //相同的关键字个数 ---> multimap为1或0
    size_type count(const key_type& x) const { 
      return t.count(x); 
    }
    //第一个>=k的元素
    iterator lower_bound(const key_type& x) const {
      return t.lower_bound(x);
    }
    //第一个>k的元素
    iterator upper_bound(const key_type& x) const {
      return t.upper_bound(x); 
    }
    //组成区间
    std::pair<iterator,iterator> equal_range(const key_type& x) const {
    return t.equal_range(x);
    }
  };

template <class Key, class Compare, class Alloc>
inline bool operator==(const multimap<Key, Compare, Alloc>& x, 
                       const multimap<Key, Compare, Alloc>& y) {
  return x.t == y.t;
}

template <class Key, class Compare, class Alloc>
inline void swap(multimap<Key, Compare, Alloc>& x, 
                 multimap<Key, Compare, Alloc>& y) {
  x.swap(y);
}

}

#endif
