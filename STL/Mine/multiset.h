

#ifndef MULTISET
#define MULTISET 

//和multiset基本一致,只是关键字重复
#include "functional.h"
#include "rbTree.h"
namespace stl{
  
  //注意默认的比较器
  template<class Key,class Compare = less<Key>,class Alloc = alloc>
    class multiset{
      public:
        //类型定义
        typedef Key key_type;
        typedef Key value_type;  //key和value是一致的
        typedef Compare key_compare;

      private:
        //key和value一致
        //最顶层的rb_tree_node里也就只有value
        //rb_tree的KeyOfValue也就原封不动返回value
        typedef rb_tree<key_type,unchanged<value_type>,value_type,key_compare,Alloc> rb_tree_type;
        rb_tree_type t;
        
        //迭代器相关-->rb_tree的迭代器
        //均为const类型 ---> 不能通过迭代器修改关键字 --> 在红黑树中有固定的次序
      public:
        typedef typename rb_tree_type::const_pointer pointer;
        typedef typename rb_tree_type::const_pointer const_pointer;
        typedef typename rb_tree_type::const_reference reference;
        typedef typename rb_tree_type::const_reference const_reference;
        typedef typename rb_tree_type::const_iterator iterator;
        typedef typename rb_tree_type::const_iterator const_iterator;
        typedef typename rb_tree_type::const_reverse_iterator reverse_iterator;
        typedef typename rb_tree_type::const_reverse_iterator const_reverse_iterator;
        typedef typename rb_tree_type::size_type size_type;
        typedef typename rb_tree_type::difference_type difference_type;
        
    //默认构造函数会function object的临时对象
    multiset():t(Compare()){}
    //显示地执行比较器
    explicit multiset(const Compare& comp) : t(comp){}
    //用迭代器范围进行初始化
    template<class InputIterator>
      multiset(InputIterator first,InputIterator last):t(Compare()){
        t.insert_equal(first,last);
      }

    template<class InputIterator>
      multiset(InputIterator first,InputIterator last,const Compare& comp):t(comp){
        t.insert_equal(first,last);
      }
    
    //会调用底层rbTree的拷贝构造函数
    multiset(const multiset<Key,Compare,Alloc>& x) : t(x.t){ }
    multiset<Key,Compare,Alloc>& operator=(const multiset<Key,Compare,Alloc>& x){
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
    void swap(multiset<Key, Compare, Alloc>& x) { t.swap(x.t); }

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
    //相同的关键字个数 ---> multiset为1或0
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
inline bool operator==(const multiset<Key, Compare, Alloc>& x, 
                       const multiset<Key, Compare, Alloc>& y) {
  return x.t == y.t;
}

template <class Key, class Compare, class Alloc>
inline void swap(multiset<Key, Compare, Alloc>& x, 
                 multiset<Key, Compare, Alloc>& y) {
  x.swap(y);
}

}

#endif
