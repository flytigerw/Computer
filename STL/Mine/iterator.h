#ifndef ITERATOR 
#define ITERATOR 

#include <cstddef>

namespace stl{
  //迭代器种类
  struct input_iterator_tag {};
  struct output_iterator_tag {};
  struct forward_iterator_tag : public input_iterator_tag{};
  struct bidirectional_iterator_tag : forward_iterator_tag{};
  struct random_access_iterator_tag : bidirectional_iterator_tag{};
  

  //方便自定义迭代器
  template<class T,class Distance> 
    struct input_iterator{
        typedef input_iterator_tag iterator_category;
        typedef T                  value_type;
        typedef T*                 pointer;
        typedef T&                 reference;
        typedef Distance           difference_type;
    };

  template<class T,class Distance> 
    struct output_iterator{
        typedef output_iterator_tag iterator_category;
        typedef T                  value_type;
        typedef T*                 pointer;
        typedef T&                 reference;
        typedef Distance           difference_type;
    };
  template<class T,class Distance> 
    struct forward_iterator{
        typedef forward_iterator_tag iterator_category;
        typedef T                  value_type;
        typedef T*                 pointer;
        typedef T&                 reference;
        typedef Distance           difference_type;
    };
  template<class T,class Distance> 
    struct bidirectional_iterator{
        typedef bidirectional_iterator_tag iterator_category;
        typedef T                  value_type;
        typedef T*                 pointer;
        typedef T&                 reference;
        typedef Distance           difference_type;
    };
  template<class T,class Distance> 
    struct random_access_iterator{
        typedef random_access_iterator iterator_category;
        typedef T                  value_type;
        typedef T*                 pointer;
        typedef T&                 reference;
        typedef Distance           difference_type;
    };

    //通用的自定义迭代器基类
    template<class Category,
             class T,
             class Distance = ptrdiff_t,
             class Pointer = T*,
             class Reference = T&>
       struct iterator{
          typedef Category iterator_category;
          typedef T        value_type;
          typedef Distance difference_type;
          typedef Pointer  pointer;
          typedef Reference reference;
    };

    //迭代器萃取机
    template<class Iterator>
      struct iterator_traits{
          typedef typename Iterator::iterator_category iterator_category;
          typedef typename Iterator::value_type value_type;
          typedef typename Iterator::Pointer pointer;
          typedef typename Iterator::Reference reference;
          typedef typename Iterator::Distance difference_type;
      };

    //针对原生指针的萃取机
    template<class T>
      struct iterator_traits<T*>{
          typedef  random_access_iterator_tag iterator_category;
          typedef  T         value_type;
          typedef  T*        pointer;
          typedef  T&        reference;
          typedef  ptrdiff_t difference_type;
      };

    template<class T>
      struct iterator_traits<const T*>{
          typedef  random_access_iterator_tag iterator_category;
          typedef  T               value_type;
          typedef  const T*        pointer;
          typedef  const T&        reference;
          typedef  ptrdiff_t       difference_type;
      };

    //快速获取迭代器的value_type,category,difference_type
    template<class Iterator>
    inline 
    typename iterator_traits<Iterator>::iterator_category 
    iterator_category(const Iterator&){
      typedef typename iterator_traits<Iterator>::iterator_category category;
      return category();
    }

    template<class Iterator>
    inline 
    typename iterator_traits<Iterator>::value_type* 
    value_type(const Iterator&){
      return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
    }

    template<class Iterator>
    inline 
    typename iterator_traits<Iterator>::difference_type* 
    distance_type(const Iterator&){
      return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
    }
    

    //distance ---> 两个迭代器之间的距离
    //在编译期完成function dispatch
    template<class InputIterator,class Distance>
    inline void 
    distance_impl(InputIterator first,InputIterator last,Distance& d,input_iterator_tag){
      for(;first!=last;++first,++d);
    }
    //随机迭代器的特性
    template<class RandomAccessIter,class Distance>
    inline void 
    distance_impl(RandomAccessIter first,RandomAccessIter last,Distance& d,random_access_iterator_tag){
      d += (last-first);
    }
    template<class Iterator,class Distance> 
    inline void 
    distance(Iterator first,Iterator last,Distance& d){
        distance_impl(first,last,d,iterator_category(first));
    }

    template<class InputIterator,class Distance>
      inline void 
      advance_impl(InputIterator& iter,Distance n,input_iterator_tag){
        while(n--) 
          ++iter;
      }

    template<class BidirectionalIterator,class Distance>
      inline void 
      advance_impl(BidirectionalIterator& iter,Distance n,bidirectional_iterator_tag){
        if(n >= 0)
          while(n--) ++iter;
        else 
          while(n++) ++iter; //加到0就为false
      }

    template<class RandomAccessIterator,class Distance>
      inline void 
      advance_impl(RandomAccessIterator& iter,Distance n,RandomAccessIterator){
        iter += n;
      }
    //advance ---> 迭代器移动n步,必须要引用
    template<class Iterator,class Distance>
      inline void 
      advance(Iterator& iter,Distance n){
        advance_impl(iter,n,iterator_category(iter));
      }

    //插入迭代器适配器
    //内含容器指针,封装出新的接口 
    //将迭代器的assign转为对容器的push_back
    template<class Container>
      class back_insert_iterator{
         protected:
           Container* container;
         public:
           //用于迭代器榨汁机的类型定义
           typedef output_iterator_tag   iterator_category;
           typedef void       value_type;
           typedef void       difference_type;
           typedef void       reference;
           typedef void       pointer;
           typedef back_insert_iterator<Container> self_type;
        explicit back_insert_iterator(Container& c) : container(&c){}
        //通过迭代器向容器里面push_back元素
        self_type& operator=(const typename Container::value_type& value){
          container->push_back(value);
          return *this;
        }
        //*,++,++(后) 可以调用，但都没有实际作用,
        self_type& operator*(){return *this;}
        self_type& operator++(){return *this;}
        self_type& operator++(int){return *this;}
      };
    //通过函数生成back_insert_iterator --> 简洁一点
    template<class Container>
      inline back_insert_iterator<Container> 
      back_insert(Container& x){
        return back_insert_iterator<Container>(x);
      }

    //照猫画虎
template <class Container>
class front_insert_iterator {
protected:
  Container* container;
public:
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  explicit front_insert_iterator(Container& x) : container(&x) {}
  front_insert_iterator<Container>&
  operator=(const typename Container::value_type& value) { 
    container->push_back(value);
    return *this;
  }
  front_insert_iterator<Container>& operator*() { return *this; }
  front_insert_iterator<Container>& operator++() { return *this; }
  front_insert_iterator<Container>& operator++(int) { return *this; }
};

template <class Container>
inline front_insert_iterator<Container> front_inserter(Container& x) {
  return front_insert_iterator<Container>(x);
}

//在任意位置点插入
template <class Container>
class insert_iterator {
protected:
  Container* container;
  typename Container::iterator iter;
public:
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;
  
  //在iter i处插入新元素
  insert_iterator(Container& x, typename Container::iterator i) 
    : container(&x), iter(i) {}
  insert_iterator<Container>&
  operator=(const typename Container::value_type& value) { 
    iter = container->insert(iter, value);
    ++iter;
    return *this;
  }
  insert_iterator<Container>& operator*() { return *this; }
  insert_iterator<Container>& operator++() { return *this; }
  insert_iterator<Container>& operator++(int) { return *this; }
};

template <class Container, class Iterator>
inline insert_iterator<Container> inserter(Container& x, Iterator i) {
  typedef typename Container::iterator iter;
  return insert_iterator<Container>(x, iter(i));
}

//正向迭代器   [begin,end)
//逆向迭代器   (rend,rbegin]
template<class Iterator>
class reverse_iterator{
  protected:
    Iterator iter;      //内部为正向迭代器
  public:
    //逆向迭代器的相关类型与底层的正向迭代器一致
    typedef  typename iterator_traits<Iterator>::iterator_category       iterator_category;
    typedef  typename iterator_traits<Iterator>::difference_type         difference_type;
    typedef  typename iterator_traits<Iterator>::pointer                 pointer;
    typedef  typename iterator_traits<Iterator>::reference               reference;
    typedef  typename iterator_traits<Iterator>::value_type              value_type;
    
    //方便操作的类型定义
    typedef Iterator iterator_type;
    typedef reverse_iterator<iterator_type> self_type;
    
  public:
    //ctors
    reverse_iterator(){}
    explicit reverse_iterator(iterator_type x) : iter(x){}
    reverse_iterator(const iterator_type& x):iter(x.iter){}
    //获取内部的正向迭代器
    iterator_type base() const {return iter;}
    
    //解引用操作:考虑当正向迭代器为end()时,逆向迭代器为rbegin(),是的确指向元素的 ---> 即最后一个 ---> 对逆向迭代器解引用应该得到正向迭代器的前一个元素 
    //同样当正向迭代器为begin时,逆向迭代器为rend() --> 不应该指向真实元素
    reference operator*() const {
      Iterator tmp = iter;  //不能改变iter,故采用一个临时变量来获取前一个元素
      return *--tmp;
    }

    pointer operator->() const{
      return &(operator*());
    }

    //++与--都是相反的操作
    self_type& operator++(){
      --iter;
      return *this;
    }   
    self_type operator++(int){
      auto tmp = *this;
      --iter;
      return tmp;
    }
    self_type& operator--() {
      ++iter;
      return *this;
    }
    self_type operator--(int) {
      self_type tmp = *this;
      ++iter;
      return tmp;
    }

    // +n --> -n
    self_type operator+(difference_type n) const{
      return  self_type(iter-n);
    }
    //支持流模式,没有const
    self_type& operator+=(difference_type n){
      iter -= n;
      return *this;
    }
    self_type operator-(difference_type n) const{
      return  self_type(iter+n);
    }
    self_type& operator-=(difference_type n) {
      iter += n;
      return  *this;
    }
    
    //[]返回可修改的左值
    reference operator[](difference_type n){
      return *(*this+n);
    }
};

//一些二元操作符的重载
template <class Iterator>
inline bool operator==(const reverse_iterator<Iterator>& x, 
                       const reverse_iterator<Iterator>& y) {
  return x.base() == y.base();
}

template <class Iterator>
inline bool operator<(const reverse_iterator<Iterator>& x, 
                      const reverse_iterator<Iterator>& y) {
  return y.base() < x.base();
}
template <class Iterator>
inline typename reverse_iterator<Iterator>::difference_type
operator-(const reverse_iterator<Iterator>& x, 
          const reverse_iterator<Iterator>& y) {
  return y.base() - x.base();
}

// n+iter ---> iter-n
template <class Iterator>
inline reverse_iterator<Iterator> 
operator+(typename  reverse_iterator<Iterator>::difference_type n,
          const reverse_iterator<Iterator>& x) {
  return reverse_iterator<Iterator>(x.base() - n);
}

}
#endif

