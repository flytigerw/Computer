
#ifndef  RBTREE
#define  RBTREE 
#include "iterator.h"
#include "alloc.h"
#include "construct.h"
#include "algorithm.h"

namespace stl{  
//节点的颜色
typedef bool rb_tree_node_color_type;
//红色为0,黑色为1
const rb_tree_node_color_type rb_tree_red = false; 
const rb_tree_node_color_type rb_tree_black = true;

//节点基类
struct rb_tree_node_base{
  typedef rb_tree_node_color_type color_type;
  typedef rb_tree_node_base*  base_node_ptr;
  
  //节点颜色，左右孩子指针，父辈指针
  color_type color;
  base_node_ptr lchild,rchild,parent;

  //最左节点
  static base_node_ptr leftmost_node(base_node_ptr x){
    while(x->lchild != nullptr)
      x = x->lchild;
    return x;
  }
  static base_node_ptr rightmost_node(base_node_ptr x){
    while(x->rchild != nullptr)
      x = x->rchild;
    return x;
  }
};

//节点类 ---> 保存的数据是泛型的
template<class T>
struct rb_tree_node : public rb_tree_node_base{
  T data;
};


//迭代器基类 只负责移动
struct rb_tree_base_iterator{
  typedef rb_tree_node_base::base_node_ptr base_node_ptr;
  //双向迭代器类型
  typedef stl::bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  base_node_ptr base_node;

  //二叉搜索树的下一个节点:考察中序遍历
  void increment(){
    if(base_node->rchild != nullptr){ //右子树的最左子节点
      while(base_node->lchild != nullptr)
        base_node = base_node->lchild;
    }else{
      //向上追溯，直到node是parent的左孩子
      //此时parent即为下一节点
      base_node_ptr p = base_node->parent; 
      while(base_node == p->rchild){
        base_node = p;
        p = p->parent;
      }
      //考虑只有root时的递增 ----> header
      if(base_node->rchild != p)
        base_node = p;
    }
  }
  
  //上一节点
  void decrement(){
    //当前迭代器为end() --> header
    if(base_node->color == rb_tree_red && base_node->parent->parent == base_node)
      base_node = base_node->rchild;

    //左子树的最右子节点
    else if( base_node->lchild != nullptr){
      while(base_node->rchild != nullptr)
        base_node = base_node->rchild;
    }else {
      //向上追溯，直到node是parent的右节点为止
      base_node_ptr p = base_node->parent;
      while(base_node == p->lchild){
        p = base_node;
        p = p->parent;
      }
      base_node = p;
    }
  }
};

//为什么要三个参数 --->  定义const iterator
template<class Value,class Ref,class Ptr>
struct rb_tree_iterator : public rb_tree_base_iterator{
    typedef Value value_type;
    typedef Ref   reference; 
    typedef Ptr   pointer; 
    //后续操作需要的类型
    typedef rb_tree_iterator<Value,Ref,Ptr> self;
    typedef rb_tree_iterator<Value,Value&,Value*> iterator;
    typedef rb_tree_iterator<Value,const Value&,const Value*> const_pointer;
    
    //带数据的节点指针
    typedef rb_tree_node<Value>* node_ptr;
    //ctors 
    rb_tree_iterator(){}
    //隐式转型
    rb_tree_iterator(node_ptr x){base_node = x;}
    rb_tree_iterator(const iterator& it){base_node = it.node;}
    
    //提取数据
    reference operator*() const {return node_ptr(base_node)->data;}
    self operator->() const {
      return &(operator*());
    }
    //++a --> 支持流模式
    self& operator++(){increment();return *this;}
    //a++ --> 不支持流模式，返回原对象拷贝
    self operator++(int){
      self tmp = *this;
      increment();
      return tmp;
    }
    self& operator--(){increment();return *this;}
    self operator--(int){
      self tmp = *this;
      decrement();
      return tmp;
    }
};   


inline bool operator == (const rb_tree_base_iterator& x,const rb_tree_base_iterator& y){
  return x.base_node == y.base_node;
}
    //左旋:可能改变root指向所以用引用
    //当前节点x左旋到y=x->rchild->lchild处
    //y接替x,与上层联系
    //y的y->lchild接到x->rchild;
 inline void 
   rb_tree_rotate_right(rb_tree_node_base* x,rb_tree_node_base*& root){
     rb_tree_node_base* y = x->rchild;
     x->rchild = y->lchild; //隐含:x->rchild一定存在
     if(y->lchild != nullptr) //隐含:y->lchild不一定存在
       y->lchild->parent = x;
     y->parent = x->parent;
     if(x == root)
       root = y;
     else if(x == x->parent->lchild)
       x->parent->lchild = y;
     else 
       x->parent->rchild = y;
     y->lchild = x;
     x->parent = y;
   }

 inline void 
  rb_tree_rotate_left(rb_tree_node_base* x,rb_tree_node_base*& root){
   rb_tree_node_base* y = x->lchild;

   x->lchild = y->rchild;
   if(y->rchild != nullptr)
     y->rchild->parent = x;
    //y开始顶替x
    y->parent = x->parent;
   if(x == root)
     root = y;
   else if(x == x->parent->rchild)
     x->parent->rchild = y;
   else 
     x->parent->lchild = y;
    y->rchild = x;
    x->parent = y;
  }

 inline void 
   insert_rebalance(rb_tree_node_base* u,rb_tree_node_base*& root){
       //上色为红色
   u->color = rb_tree_red;
   while(u != root && u->parent->color == rb_tree_red){
     //连续两个红色，失衡
     auto pu = u->parent;
     auto gu = pu->parent;
     if(pu == gu->lchild){ //L型
       auto gur = gu->rchild;
       if(gur && gur->color == rb_tree_red){ //Lxr型
         //将gu的右孩子和pu调为黑色
         gur->color= rb_tree_black;
         pu->color = rb_tree_black;
        //将gu调为红色
         gu->color = rb_tree_red;
         //让u = gu向上继续调整
         u = gu;
       }else{  
         if(u == pu->rchild) //Lrb型
             rb_tree_rotate_right(pu,root);
         pu->color = rb_tree_black;
         gu->color = rb_tree_red;
         rb_tree_rotate_left(gu,root);
       }
     }else{//R型
       auto gul = gu->lchild;
       if(gul && gul->color == rb_tree_red){ //Rxr型
           //将pu和gul调为黑色
           pu->color = rb_tree_black;
           gul->color = rb_tree_black;
           //将gu调为红色
           gu->color = rb_tree_red;
           //向上继续调整
           u = gu;
       }else{  //Rxb型
         if(u == pu->lchild) //Lrb型
             rb_tree_rotate_left(pu,root);
         pu->color = rb_tree_black;
         gu->color = rb_tree_red;
         rb_tree_rotate_right(gu,root);
       }
     }
   }
  root->color = rb_tree_black;
 }

 inline rb_tree_node_base*
   erase_rebalance(rb_tree_node_base* p,rb_tree_node_base*& root,rb_tree_node_base*& leftmost,rb_tree_node_base*& rightmost){

     
     //p指向要被删除的节点
     auto y = p;
     rb_tree_node_base* x = nullptr,*x_parent = nullptr;
    
     //只有一颗子树时
     if(y->lchild == nullptr)
       x = y->rchild;
     else{
       if(y->rchild == nullptr)
         x = y->lchild;
       else{ // 左右子树都不为空
         //找到右子树的最左子节点
         y = y->rchild;
         while(y->lchild)
          y = y->lchild;
         x = y->rchild;
       }
       if(y != p){       //两颗子树的情况
         //y为右子树的最左子节点
        //x=y->rchild
         //y将顶替被删除的节点
         //y接管p的左子树
         p->lchild->parent = y;
         y->lchild = p->lchild;
           
         //p->rchild->lchild不为空时
         //处理y的右子树
         if(y != p->rchild){  
           //右子树存在
           if(x)
             x->parent = y->parent;
           y->parent->lchild = x;
           //接管p的右子树
           y->rchild = p->rchild;
           p->rchild->parent = y;
         }
           
         //若删除的是根节点
         //让y接管p的上面部分
         if(p == root)
           root = y;
         else if(p->parent->lchild == p)
           p->parent->lchild = y;
         else 
           p->parent->rchild = y;
         y->parent = p->parent;

         auto tmp_color = y->color;
         y->color = p->color;
         p->color = tmp_color;

         y = p;
       }
       else{      //y = p ---> 叶子节点和一颗子树的情况  
         //x将顶替p
         //非叶子节点
         if(x)
           x->parent = y->parent;
         //可能根节点也是叶子节点
         if(root == p)
           root = x;
         else if(p->parent->lchild == p)
           p->parent->lchild = x;
         else 
           p->parent->rchild = x;
       }
       //leftmost和rightmost可能被erase
      //更新leftmost和rightmost
       if(leftmost == p){
        if(p->rchild == nullptr)
          leftmost = p->parent;
        else 
          leftmost = rb_tree_node_base::leftmost_node(x);
       }
       if(rightmost == 0){
         if(p->lchild == nullptr)
           rightmost = p->parent;
         else 
           rightmost = rb_tree_node_base::rightmost_node(x);
       }
     }
       //现在x顶替被删除的节点的位置
       x_parent = x->parent;
       //删除节点为红色不会失衡
       //调整可能失衡的情况
       if(y->color == rb_tree_black){
         //若x为根或者x的颜色为红色退出循环
         while(x != root && (x ==nullptr || x->color == rb_tree_black)){
           if(x == x_parent->rchild){ //R型
             auto brother = x_parent->lchild;
             if(brother->color == rb_tree_red){ //Rr型，先统一处理
               brother->color = rb_tree_black;
              x_parent->color = rb_tree_red;
              rb_tree_rotate_left(x_parent,root);
              brother = x_parent->lchild;
             }
             //子树不存在，或者存在的子树为黑色
             //Rr0和Rb0型
            if((brother->rchild == nullptr ||brother->rchild->color == rb_tree_black) && (brother->lchild== nullptr || brother->lchild->color == rb_tree_black)){
             brother->color = rb_tree_red;
             x = x_parent;
             x_parent = x_parent->parent;
            }else{
            //Rr1II型,Rb1I型
              if(brother->lchild == nullptr || brother->lchild->color == rb_tree_black){
                if(brother->rchild)
                 brother->rchild->color = rb_tree_black;
                brother->color = rb_tree_red;
                rb_tree_rotate_right(brother,root);
                brother = x_parent->lchild;
              }
          //Rr1I型,RrII型,Rb1II型,Rb2型
             brother->color = x_parent->color;
             x_parent->color = rb_tree_black;
             if(brother->lchild)
              brother->lchild->color = rb_tree_black;
             rb_tree_rotate_left(x_parent,root);
             break;
          }
      }else{  //L型 --> 对称
        auto brother = x_parent->rchild;
        if(brother->color == rb_tree_red){ //Rr型，现在统一处理
          brother->color = rb_tree_black;
          x_parent->color = rb_tree_red;
          rb_tree_rotate_right(x_parent,root);
          brother = x_parent->lchild;
        }
       if((brother->rchild == nullptr ||brother->rchild->color == rb_tree_black) && (brother->lchild== nullptr || brother->lchild->color == rb_tree_black)){
          brother->color = rb_tree_red;
          x = x_parent;
          x_parent = x_parent->parent;
          }else{
            if(brother->rchild == nullptr || brother->rchild->color == rb_tree_black){
              if(brother->lchild)
                brother->lchild->color = rb_tree_black;
              brother->color = rb_tree_red;
              rb_tree_rotate_right(brother,root);
              brother = x_parent->rchild;
            }
            brother->color = x_parent->color;
            x_parent->color = rb_tree_black;
            if(brother->rchild)
              brother->rchild->color = rb_tree_black;
            rb_tree_rotate_right(x_parent,root);
            break;
        }
      }
    }
      //若是因为x为红色退出循环--->有两个连续的红色 --> 失衡
      if(x) 
        x->color = rb_tree_black;
    }
  return y;
 }

//KeyOfValue是function obejct.可以提取出Value对应的Key
//Compare 是Key的比较器 ---> <=比较器
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc = alloc> 
class rb_tree{
  protected:
    typedef rb_tree_node_base*  base_node_ptr;
    typedef rb_tree_node<Value> node;  
    typedef rb_tree_node<Value>* node_ptr;
    typedef simple_alloc<rb_tree_node<Value>,Alloc> rb_tree_node_allocator;
    typedef rb_tree_node_color_type color_type;

  public:
    //用于迭代器的萃取以及一些类型使用
    typedef Key    key_type;
    typedef Value  value_type;
    typedef value_type*  pointer;
    typedef value_type&  reference;
    typedef const value_type& const_reference;
    typedef const pointer& const_pointer;
    typedef size_t size_type;
    typedef ptrdiff_t    difference_type;
    typedef rb_tree<Key,KeyOfValue,Value,Compare,Alloc> self_type;

    //迭代器相关
  public:
    typedef rb_tree_iterator<value_type,reference,pointer> iterator;
    typedef rb_tree_iterator<value_type,const_reference,const_pointer> const_iterator;
    typedef reverse_iterator<const_iterator> const_reverse_iterator;
    typedef reverse_iterator<iterator> reverse_iterator;

    //数据成员
  protected:
    size_type node_count;   //节点数量
    Compare   key_compare;  //键值比较准则
    node_ptr  header;       //与root互为父节点 --> 也是尾后迭代器end
  
    //内存操作
  protected:
    //获取一个节点的内存
    node_ptr get_node_memory(){
      return rb_tree_node_allocator::allocate();
    }
    //归还一个节点的内存
    void return_node_memory(node_ptr p){
      rb_tree_node_allocator::deallocate(p);
    }
    //构造一个节点
    node_ptr construct_node(const value_type& x){
      node_ptr tmp = get_node_memory();
      try{
        //important
        construct_node(&tmp->data,x);  
      }catch(...){
        return_node_memory(tmp);
        throw ;
      }
      return tmp;
    }
    //析构一个节点
    void destroy_node(node_ptr p){
      destroy(&p->data);
      return_node_memory(p);
    }

    //节点的复制
    node_ptr clone_node(node_ptr x){
      auto node = construct_node(x->data);
      node->color = x->color;
      node->lchild = node->rchild = nullptr;
      return node;
    }
   
    //辅助函数
  protected:
    //根节点
    node_ptr& get_root() const {return (node_ptr*)header->parent;}
    node_ptr& get_leftmost() const {return header->lchild;}
    node_ptr& get_rightmost() const {return header->lchild;}
   //以节点x为根的子树的极值
    static node_ptr get_minimum(node_ptr x){
      return (node_ptr)rb_tree_node_base::leftmost_node(x);
    }
    static node_ptr get_maximum(node_ptr x){
      return (node_ptr)rb_tree_node_base::rightmost_node(x);
    }
    static const Key& key(base_node_ptr x){
      return KeyOfValue()(((node_ptr)x)->data);
    }
    //私有辅助函数
  private:

    void header_init(){
      header = get_node_memory();
      header->color = rb_tree_red;
      header->lchild = header;
      header->rchild = header;
      header->parent = nullptr;
    }
    iterator insert_impl(base_node_ptr,const value_type&);
    void erase_impl(node_ptr );
    node_ptr copy_impl(node_ptr,node_ptr);
  
    //ctors and dctors
  public:
  
    //含有默认的比较器
    rb_tree(const Compare& comp = Compare()):node_count(0),key_compare(comp){header_init();}
    
    //copy ctor 
    rb_tree(const self_type& x){
      header_init();
      if(x.node_count != 0){
        //header->parent 为root
        header->parent = copy_impl(x.header->parent,header);
        header->lchild = get_minimum(header->parent);
        header->rchild = get_maximum(header->parent);
      }
      node_count = x.node_count;
      key_compare = x.key_compare;
    }
    
    //dctor 
    ~rb_tree(){
      clear();
      return_node_memory(header);
    }
    //copy = 
    self_type&
      operator = (const self_type& x ){
        if(this != &x){
          clear();
          if(x.node_count != 0){
           header->parent = copy_impl(x.header->parent,header);
           header->lchild = get_maximum(header->parent);
           header->rchild = get_maximum(header->parent);
          }
          node_count == x.node_count;
          key_compare = x.key_compare;
        }
        return *this;
      }
  
    //对外接口
  public:
    Compare key_comp() const {return key_compare;}
    iterator begin() {return get_leftmost();}
    const_iterator cbegin() const {return get_leftmost();}
    iterator end() const {return header;} 
    const_iterator cend() const {return end;}
    bool empty() const {return node_count == 0;}
    size_type size() const {return node_count;}
    //special
    size_type max_size() const {return size_type(-1);}
    //交换header指针指向，数据成员的值
    void swap(rb_tree<Key,KeyOfValue,Value,Compare,Alloc>& t){
      std::swap(header,t.header);
      std::swap(node_count,t.node_count);
      std::swap(key_compare,t.key_compare);
    }

   //插入新节点
    std::pair<iterator,bool> insert_unique(const value_type&);
    iterator insert_unique(iterator position,const value_type&);
    iterator insert_equal(const value_type&);
    iterator insert_equal(iterator position,const value_type&);
   //插入范围
   template<class InputIterator>
   void insert_unique(InputIterator first,InputIterator last);
   template<class InputIterator>
   void insert_equal(InputIterator first,InputIterator last);
  
   //删除节点
   void erase(iterator position);
   size_type erase(const key_type&x);
   //删除范围
   void erase(iterator first,iterator last);
   void erase(const key_type* first,const key_type* last);
   //删除所有
   void clear(){
       if(node_count != 0){
        erase_impl(get_root());
        header->lchild = header->rchild = header;
        header->parent = nullptr; //root 
        node_count = 0;
     }
   }

   //查找 相关
   iterator find(const key_type& x);
   const_iterator find(const key_type& x) const;
   iterator lower_bound(const key_type& x);
   const_iterator lower_bound(const key_type& x)const;
   iterator upper_bound(const key_type& x);
   const_iterator upper_bound(const key_type& x)const;
   std::pair<iterator,iterator>equal_range(const key_type& x);
   std::pair<const_iterator,const_iterator>eqaul_range(const key_type& x)const;
   size_type count(const key_type& x) const;
};

//操作符重载
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
inline bool 
operator==(const rb_tree<Key,KeyOfValue,Value,Compare,Alloc>& x,const rb_tree<Key,KeyOfValue,Value,Compare,Alloc>& y){
  return x.size() == y.size() && equal(x.cbegin(),x.cend(),y.cbegin());
}

//假设x为新值插入点,xp为其父节点
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename  rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_impl(base_node_ptr xp,const Value& v){
    node_ptr new_node = construct_node(v);
    new_node->rchild = new_node->lchild = nullptr;
    new_node->parent = xp;   //若插入的为根节点，则其父节点就为header
    //非header 比较成功 ---> 插入unique节点
    if(xp==header || key_compare(KeyOfValue()(v),key(xp))) {
        xp->lchild = new_node;
        if(xp == header){   //只有一个节点时,header的三个指针都指向同一个
          header->parent = new_node;
          header->rchild = new_node;
        }else if(xp == header->lchild)
          header->lchild = new_node;  //保证header的左孩子指向最小的节点
    }else{ //非header,比较失败 --> 插入右子树
      xp->rchild = new_node;
      if(xp == header->rchild)
        header->rchild = new_node;
    }
    ++node_count;
    //可能失衡进行调整
    rb_tree_insert_rebalance(new_node,header->root);
    //返回指向新节点的iterator
    return iterator(new_node);
}

//键值可重复
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename  rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_equal(const Value& v){
    //先找到插入点
    auto x = header;
    auto xp= header->parent;
    while(x != nullptr){
      xp = x;
      x = (key_compare(KeyOfValue()(v),key(x))) ? x->lchild : x->rchild;
    }
    return insert_impl(xp,v);
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
std::pair<typename  rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator,bool>
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_unique(const Value& v){

    auto xp= header->parent;
    auto x = header;
    bool is_less = true;
    while(x != nullptr){
      xp = x;
      is_less = key_compare(KeyOfValue()(v),key(x));
      x = (is_less) ? x->lchild : x->rchild;
    }
    iterator i = iterator(xp);
    
    //考虑如果v和红黑树的节点n的key重复
    //x指向其待插入位置-->为空
    //xp为x的父节点
    //若n为叶子节点,则is_less最后为false 
    //若n为内部节点,is_less最后为true
    //此时中序遍历的情况为: n,xp ---> --xp ==  n
    
    if(is_less){
      if(i == begin())
        return std::make_pair(insert_impl(xp,v),true);
      else 
        --i;
    }
   if(key_compare(key(i.base_node),KeyOfValue()(v))) 
        return std::make_pair(insert_impl(xp,v),true);
   //插入失败,返回重复的节点
   return std::make_pair(i,false);
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename  rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_unique(iterator position,const Value& v){
  //
  if(position.base_node == header->lchild){//begin
      //不为空,严格弱序,直接调用insert_impl更快
      if(size() > 0 && key_compare(KeyOfValue()(v),key(position.base_node)))
        return insert_impl(position.base_node,v);
      else 
        return insert_unique(v).first; //insert_unique有去重功能,但比insert_impl慢
  }else if(position.base_node == header){ //end()
    if(key_compare(key(header->rchild),KeyOfValue()(v)))
      return insert_impl(header->rchild,v);
    else 
      return insert_unique(v).first;
  }else{
    iterator before = position; 
    --before;
    //v的值处于before.key和position.key之间
    //过小过大都要进行去重检测
    if (key_compare(key(before.node), KeyOfValue()(v))
        && key_compare(KeyOfValue()(v), key(position.node))){
      if (before.base_node->rchild == nullptr)
        return insert_impl( before.base_node, v); 
      else
        return insert_impl( position.base_node, v);
    }
    else
      return insert_unique(v).first;
  }
}
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename  rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_equal(iterator position,const Value& v){
  //类似上一个函数
  if(position.base_node == header->lchild){//begin
      //不为空,严格弱序,直接调用insert_impl更快
      if(size() > 0 && key_compare(KeyOfValue()(v),key(position.base_node)))
        return insert_impl(position.base_node,v);
      else 
        return insert_equal(v);
  }else if(position.base_node == header){ //end()
    //v >= rightmost
    if(!key_compare(KeyOfValue()(v)),key(header->rchild))
      return insert_impl(header->rchild,v);
    else 
      return insert_equal(v);
  }else{
    iterator before = position; 
    --before;
    if (!key_compare(KeyOfValue()(v), key(before.node))
        && !key_compare(key(position.node), KeyOfValue()(v))){
      if (before.base_node->rchild == nullptr)
        return insert_impl( before.base_node, v); 
      else
        return insert_impl( position.base_node, v);
    }
    // first argument just needs to be non-null 
    else
      return insert_equal(v);
  }
 }

 


template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
template<class InputIterator>
void 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_equal(InputIterator first,InputIterator last){
  for(;first != last;first++)
    insert_equal(*first);
}




template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
template<class InputIterator>
void 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
insert_unique(InputIterator first,InputIterator last){
  for(;first != last;first++)
    insert_unique(*first);
}


//根据位置删除
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
inline void 
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
erase(iterator position){
    node_ptr x=erase_rebalance(position.base_node,
                            header->parent,
                            header->lchild,
                            header->rchild);
    destroy_node(x);
    --node_count;
}

//根据关键字删除  ---> 可能含有多个相同的
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::size_type
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
erase(const Key& k){
        auto p = equal_range(k);
        size_type n = 0;
        distance(p.first,p.second,n);
        erase(p.first,p.second);
        return n;
}



//将以x为根的子树拷贝给y
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::node_ptr
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
copy_impl(node_ptr x,node_ptr p){
    node_ptr top = clone_node(x);
    top->parent = p;
    try{
      if(x->rchild)
        top->rchild = copy_impl(x->rchild,top);
      p = top;
      x = x->rchild;
      while(x){
        node_ptr y = clone_node(x);
        p->lchild = y;
        y->parent = p;
        if(x->rchild)
          y->rchild = copy_impl(x->rchild,y);
        p = y;
        x = x->lchild;
      }
    }catch(...){
      erase_impl(top);
      throw ;
    }
    return top;
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
void rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
erase_impl(node_ptr x){
  while(x){
    //先不断进入右子树
    erase_impl(x->rchild);
    //为空时，返回上一层
    //进入左子树
    node_ptr left = x->lchild;
    //销毁该层的x
    destroy_node(x);
    //再顺着左子树进入右子树
    x = left;
  }
}

//删除范围内的元素 --> 迭代器指示范围
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
void rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
erase(iterator first,iterator last){
    if(first == begin() && last == end()) 
      clear();
    else 
      while(first != last)
        erase(first++);    //++操作可能会向上追溯多次
}


//关键字组成的范围数组
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
void rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
erase(const Key* first,const Key* last){
    while(first != last)
      erase(*first++);
}


template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
find(const Key& k){
      node_ptr xp =header;
      node_ptr x = header->parent;
      while(x){
        // x>=k,进入左子树
        if(!key_compare(key(x),k)){
          xp = x;         //若最终找到,xp.key == k
                          //若没找到 ---> 后续会因为x.key严格偏小的选项不断进入右子树直到为空 --> xp.key > k
                          //又或者偏大不断进入左子树直至空 --> xp.key > k
          x = x->lchild;
        }else 
          x = x->rchild;
      }
      iterator i = iterator(xp);
      // 若k<xp.key --> 没找到
      return (i == end() || key_compare(k,key(i.base_node))) ? end() : i;
}


template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::const_iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
find(const Key& k) const {
      node_ptr xp =header;
      node_ptr x = header->parent;
      while(x){
        // x>=k,进入左子树
        if(!key_compare(key(x),k)){
          xp = x;       
          x = x->lchild;
        }else 
          x = x->rchild;
      }
      const_iterator i = const_iterator(xp);
      return (i == end() || key_compare(k,key(i.base_node))) ? end() : i;
}

//对关键字为key的节点进行计数
//寻找node x --> 在排序序列中,x.key是第一个>=key的

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::size_type
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
count(const Key& k) const {
  auto p = eqaul_range(k);
  size_type n =0;
  distance(p.first,p.second,n);
  return n;
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
lower_bound(const Key& k){
          node_ptr xp = header;
          node_ptr x = header->parent;
          while(x){
            //x.key >= k
            if(!key_compare(key(x),k)){
              xp = x;
              x = x->lchild;
            }else 
              x = x->rchild;
          }
          return iterator(xp);
}

//寻找node x --> 在排序序列中,x.key是第一个>key的
//那么lower_bound和upper_bound组成区间 [>=key,>key]可以在其中对关键字为key的node计数
//计数结果可能为0
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
upper_bound(const Key& k){
          node_ptr xp = header;
          node_ptr x = header->parent;
          while(x){
            // k < x
            if(key_compare(k,key(x))){
              xp = x;
              x = x->lchild;
            }else 
              x = x->rchild;
          }
          return iterator(xp);
}
//显示得到这一区间  --> [)型
template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
inline std::pair<typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator,typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::iterator>
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
equal_range(const Key& k){
  return std::make_pair(lower_bound(k),upper_bound(k));
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::const_iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
upper_bound(const Key& k)const{
          node_ptr xp = header;
          node_ptr x = header->parent;
          while(x){
            // k < x
            if(key_compare(k,key(x))){
              xp = x;
              x = x->lchild;
            }else 
              x = x->rchild;
          }
          return const_iterator(xp);
}

template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::const_iterator
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
lower_bound(const Key& k) const {
          node_ptr xp = header;
          node_ptr x = header->parent;
          while(x){
            //x.key >= k
            if(!key_compare(key(x),k)){
              xp = x;
              x = x->lchild;
            }else 
              x = x->rchild;
          }
          return const_iterator(xp);
}


template<class Key,class KeyOfValue,class Value,class Compare,class Alloc>
inline std::pair<typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::const_iterator,typename rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::const_iterator>
rb_tree<Key,KeyOfValue,Value,Compare,Alloc>::
eqaul_range(const Key& k)const{
    return std::make_pair(lower_bound(k),upper_bound(k));
}



}
#endif 
