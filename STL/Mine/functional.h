
#ifndef FUNCTORS
#define FUNCTORS 

namespace stl{

//一元仿函数的类型定义
//任何自定义的一元仿函数都应继承这一类型基类 ---->  就像自定义的迭代器需要继承迭代器类型基类 
//类型基类中的类型信息则可以被adapters获取   ---->  类似迭代器的信息可以被iterator_traits获取
template<class Arg,class Result>
struct unary_function{
  typedef Arg argument_type;
  typedef Result result_type;
};

//二元仿函数类型基类
template<class Arg1,class Arg2,class Result>
struct binary_function{
  typedef Arg1 first_argument_type;
  typedef Arg2 second_argument_type;
  typedef Result result_type;
};


//unchanged 用于set中  ----> key与value都是一样的
//其是为了和map使用的select_first作对比
//map的value_type为pair,要挑出key来使用
template<class T>
struct unchanged : public unary_function<T,T>{
    const T& operator()(const T& x) const {return x;}  //原封不动的返回
};

template<class Pair>
struct select_first : public unary_function<Pair,typename Pair::first_tye>{
  const typename Pair::first_tye& operator()(const Pair& x) const {
    return x.first;
  }
};

//x+y 
template<class T>
struct plus:binary_function<T,T,T>{
  T operator()(const T& x,const T&y) const{
    return x+y;
  }
};

//x-y 
template<class T>
struct minus: public binary_function<T,T,T>{
  T operator() (const T&x,const T&y) const {
    return x - y;
  }
};

//x*y 
template<class T>
struct multiplies : public binary_function<T,T,T>{
  T operator() (const T&x,const T&y) const {
    return x*y;
  }
};

// x/y
template<class T>
struct divides: public binary_function<T,T,T>{
  T operator() (const T&x,const T&y) const {
    return x/y;
  }
};

// x%y
template<class T>
struct modulus: public binary_function<T,T,T>{
  T operator() (const T&x,const T&y) const {
    return x%y;
  }
};

// x == y
template<class T>
struct euqal_to: public binary_function<T,T,T>{
  bool operator() (const T&x,const T&y) const {
    return x == y;
  }
};
// -x 
template<class T>
struct negate: public unary_function<T,T>{
  T operator() (const T&x) const {
    return -x;
  }
};

// x == y
template<class T>
struct equal_to : public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x == y;
  }
};

// x != y
template<class T>
struct not_equal_to : public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x != y;
  }
};

// x > y
template<class T>
struct greater : public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x > y;
  }
};

// x < y
template<class T>
struct less: public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x < y;
  }
};

// x >= y
template<class T>
struct greater_equal_to: public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x >= y;
  }
};

// x <= y
template<class T>
struct less_equal_to: public binary_function<T,T,T>{
  bool operator() (const T& x,const T& y){
    return x <= y;
  }
};

// x && y 
template <class T>
struct logical_and : public binary_function<T, T, bool> {
    bool operator()(const T& x, const T& y) const { return x && y; }
};

// x || y
template <class T>
struct logical_or : public binary_function<T, T, bool> {
    bool operator()(const T& x, const T& y) const { return x || y; }
};

// !x
template <class T>
struct logical_not : public unary_function<T, bool> {
    bool operator()(const T& x) const { return !x; }
};

//adapters ---> 对function object进行组合,重新封装,进而产生更复杂的表达式
//内含一个谓词对象A,封装成新的谓词B.对A的接口进行修饰,成为B的接口，暴露给用户


//一元表达式的否定: unary_negate<logical_not<bool>> a;
template<class Predicate>
class unary_negate 
  : public unary_function<typename Predicate::argument_type,bool>{
  
    protected:
      Predicate pred;
    public:
      explicit unary_negate(const Predicate& x) : pred(x){}
      bool operator()(const typename Predicate::argument_type& x) const {
        return !pred(x);
      }
};

//通过函数调用更为简洁 
//not1(logical_not<boo>>)
template<class Predicate>
inline unary_negate<Predicate> not1(const Predicate& pred){
  return unary_negate<Predicate>(pred);
}

//二元表达式的否定
template<class Predicate>
class binary_negate 
:public binary_function<typename Predicate::first_argument_type,typename Predicate::second_argument_type,bool>{
    
  protected:
    Predicate pred;
  public:
    binary_negate(const Predicate& p) : pred(p){}
    bool operator()(const typename Predicate::first_argument_type& x,const typename Predicate::second_argument_type& y) const{
      return !pred(x,y);
    }
};

template<class Predicate>
inline binary_negate<Predicate> not2(const Predicate& pred){
  return binary_negate<Predicate>(pred);
}

//绑定二元表达式的第二个参数
//比如表达 x+2 ---> bind_second(minus<int>,2) f; --> 调用f(3) --> 3+2 ----> 只要一个参数
template<class Operation>
class bind_second 
:public unary_function<typename Operation::first_argument_type,typename Operation::result_type>{

  protected:
    Operation op;
    typename Operation::second_argument_type second_argument;

  public:
    bind_second(const Operation& x,const typename Operation::second_argument_type& y ) :op(x),second_argument(y){}

    typename Operation::result_type 
      operator()(const typename Operation::first_argument_type& first_argument){
        return op(first_argument,second_argument);
      }
};

template<class Operation>
class bind_first 
:public unary_function<typename Operation::second_argument_type,typename Operation::result_type>{

  protected:
    Operation op;
    typename Operation::first_argument_type first_argument;

  public:
    bind_first(const Operation& x,const typename Operation::first_argument_type& y ) :op(x),first_argument(y){}

    typename Operation::result_type 
      operator()(const typename Operation::second_argument_type& second_argument){
        return op(first_argument,second_argument);
      }
};

//函数版本
template<class T,class Operation>
inline bind_first<Operation>
bind_first_func(const Operation& x,const T& y){
  typedef typename Operation::first_argument_type first_arg_type;
  return bind_first<Operation>(x,first_arg_type(y));
}

template<class T,class Operation>
inline bind_second<Operation>
bind_second_func(const Operation& x,const T& y){
  typedef typename Operation::second_argument_type second_arg_type;
  return bind_second<Operation>(x,second_arg_type(y));
}




//两个一元表达式的嵌套组合
//比如f(g(x)) g(x) = !x  f(y) = -y
//表达为:unary_compose<logical_not<int>,negate<int>> h(op1,op2);调用h(1)
template<class Operation1,class Operation2>
class unary_compose 
:public unary_function<typename Operation2::argument_type,typename Operation1::result_type>{

  protected:
    Operation1 op1;
    Operation1 op2;
  public:
    unary_compose(const Operation1& x,const Operation2& y) : op1(x),op2(y){}
    typename Operation1::result_type 
      operator()(const typename Operation2::argument_type& x) const{
        return op1(op2(x));
      }
};

template <class Operation1, class Operation2>
inline unary_compose<Operation1, Operation2> compose1(const Operation1& op1, 
                                                      const Operation2& op2) {
  return unary_compose<Operation1, Operation2>(op1, op2);
}

//三个二元表达式的组合
//比如: f(x) + g(x) 
template<class Operation1,class Operation2,class Operation3>
class ternary_compose
:public unary_function<typename Operation2::argument_type,typename Operation1::result_type>{

  protected:
    Operation1 op1;
    Operation2 op2;
    Operation3 op3;
  public:
    ternary_compose(const Operation1& x,const Operation2& y,const Operation3& z): op1(x),op2(y),op3(z){}
    typename Operation1::result_type 
      operator()(const typename Operation2::argument_type& x) const{
        return op1(op2(x)+op3(x));
      }
};

template <class Operation1, class Operation2, class Operation3>
inline ternary_compose<Operation1, Operation2, Operation3> 
compose3(const Operation1& op1, const Operation2& op2, const Operation3& op3) {
  return ternary_compose<Operation1, Operation2, Operation3>(op1, op2, op3);
}


//将一元函数指针包装为function object ---> 具有配接能力
template<class Arg,class Result>
class pointer_to_unary_function 
:public unary_function<Arg,Result>{
    protected:
      Result (*ptr)(Arg);
    public:
      pointer_to_unary_function(){}
      explicit pointer_to_unary_function(Result (*x)(Arg)):ptr(x){}
      Result operator()(Arg x) const{return ptr(x);}
};

template<class Arg,class Result>
inline pointer_to_unary_function<Arg,Result>
ptr_fun(Result (*x) (Arg)){
  return pointer_to_unary_function<Arg,Result>(x);
}
//二元函数
template <class Arg1, class Arg2, class Result>
class pointer_to_binary_function : public binary_function<Arg1, Arg2, Result> {
protected:
    Result (*ptr)(Arg1, Arg2);
public:
    pointer_to_binary_function() {}
    explicit pointer_to_binary_function(Result (*x)(Arg1, Arg2)) : ptr(x) {}
    Result operator()(Arg1 x, Arg2 y) const { return ptr(x, y); }
};

template <class Arg1, class Arg2, class Result>
inline pointer_to_binary_function<Arg1, Arg2, Result> 
ptr_fun(Result (*x)(Arg1, Arg2)) {
  return pointer_to_binary_function<Arg1, Arg2, Result>(x);
}


//将成员函数封装为function onject 
//若成员函数为虚函数
//则可以通过泛型算法,调用用虚函数封装后的function object,来实现多态调用
//比如vector<shape*> --> shape为基类,vector里面还有Rect*,Circle*等各种具体的图形,且display()为继承体系的虚函数

//成员函数指针的定义
// 返回类型 (类名::*name)(参数) = &类名::函数名
//调用方式
//  (实例指针->*name)(参数)

//无参，通过pointer调用的non-const成员函数
template<class Result,class T>
class mem_fun_t : public unary_function<T*,Result>{

    private:
      Result (T::*f)();
    public:
      //届时传递 &xx::func
      explicit mem_fun_t(Result (T::*pf)()) : f(pf){}
      Result operator()(T* p) const{
        return (p->*f)();
      }
};

//const成员函数
template<class Result,class T>
class const_mem_fun_t : public unary_function<const T*,Result>{

    private:
      Result (T::*f)()const;
    public:
      explicit const_mem_fun_t(Result (T::*pf) ()const) : f(pf){}
      Result operator()(const T* p) const{
        return (p->*f)() ;
      }
};

//引用调用
template<class Result,class T>
class mem_fun_ref_t : public unary_function<T,Result>{
    private:
      Result (T::*f)();
    public:
      //届时传递 &xx::func
      explicit mem_fun_ref_t(Result (T::*pf)()) : f(pf){}
      Result operator()(T& p)const{
        return (p.*f)();
      }
};

//引用调用,const成员函数
template<class Result,class T>
class const_mem_fun_ref_t : public unary_function<T,Result>{
    private:
      Result (T::*f)()const;
    public:
      explicit const_mem_fun_ref_t(Result (T::*pf)()const) : f(pf){}
      Result operator()(const T& p) const{
        return (p.*f)();
      }
};

//含有参数,指针调用
template<class Result,class T,class Arg>
class mem_fun1_t : binary_function<T*,Arg,Result>{

    private:
      Result (T::*f)(Arg);
    public:
      explicit mem_fun1_t(Result (T::*pf)()) : f(pf){}
      Result operator()(T* p,Arg x)const {
        return (p->*f)(x);
      }
};

//const成员函数
template<class Result,class T,class Arg>
class const_mem_fun1_t : binary_function<T*,Arg,Result>{

    private:
      Result (T::*f)(Arg) const;
    public:
      explicit const_mem_fun1_t(Result (T::*pf)()const) : f(pf){}
      Result operator()(const T* p,Arg x)const {
        return (p->*f)(x);
      }
};

//含有参数,引用调用
template<class Result,class T,class Arg>
class mem_fun1_ref_t : public binary_function<T,Arg,Result>{
    private:
      Result (T::*f)(Arg);
    public:
      explicit mem_fun1_ref_t(Result (T::*pf)(Arg)) : f(pf){}
      Result operator()(T& p,Arg x)const{
        return (p.*f)(x);
      }
};

//const 成员函数
template<class Result,class T,class Arg>
class const_mem_fun1_ref_t : public binary_function<T,Arg,Result>{
    private:
      Result (T::*f)(Arg) const;
    public:
      explicit const_mem_fun1_ref_t(Result (T::*pf)(Arg)const) : f(pf){}
      Result operator()(const T& p,Arg x)const{
        return (p.*f)(x);
      }
};


//偏特化版本

//返回void
template <class T>
class mem_fun_t<void, T> : public unary_function<T*, void> {
public:
  explicit mem_fun_t(void (T::*pf)()) : f(pf) {}
  void operator()(T* p) const { (p->*f)(); }
private:
  void (T::*f)();
};

//返回void,const成员函数
template <class T>
class const_mem_fun_t<void, T> : public unary_function<const T*, void> {
public:
  explicit const_mem_fun_t(void (T::*pf)() const) : f(pf) {}
  void operator()(const T* p) const { (p->*f)(); }
private:
  void (T::*f)() const;
};

//返回void,引用调用
template <class T>
class mem_fun_ref_t<void, T> : public unary_function<T, void> {
public:
  explicit mem_fun_ref_t(void (T::*pf)()) : f(pf) {}
  void operator()(T& r) const { (r.*f)(); }
private:
  void (T::*f)();
};

//返回void,引用调用,const成员函数
template <class T>
class const_mem_fun_ref_t<void, T> : public unary_function<T, void> {
public:
  explicit const_mem_fun_ref_t(void (T::*pf)() const) : f(pf) {}
  void operator()(const T& r) const { (r.*f)(); }
private:
  void (T::*f)() const;
};

//返回void,指针调用,含有参数
template <class T, class A>
class mem_fun1_t<void, T, A> : public binary_function<T*, A, void> {
public:
  explicit mem_fun1_t(void (T::*pf)(A)) : f(pf) {}
  void operator()(T* p, A x) const { (p->*f)(x); }
private:
  void (T::*f)(A);
};

//返回void,指针调用,含有参数,const成员函数
template <class T, class A>
class const_mem_fun1_t<void, T, A> : public binary_function<const T*, A, void> {
public:
  explicit const_mem_fun1_t(void (T::*pf)(A) const) : f(pf) {}
  void operator()(const T* p, A x) const { (p->*f)(x); }
private:
  void (T::*f)(A) const;
};

//返回void,引用调用,含有参数
template <class T, class A>
class mem_fun1_ref_t<void, T, A> : public binary_function<T, A, void> {
public:
  explicit mem_fun1_ref_t(void (T::*pf)(A)) : f(pf) {}
  void operator()(T& r, A x) const { (r.*f)(x); }
private:
  void (T::*f)(A);
};

//返回void,引用调用,含有参数,const成员函数
template <class T, class A>
class const_mem_fun1_ref_t<void, T, A> : public binary_function<T, A, void> {
public:
  explicit const_mem_fun1_ref_t(void (T::*pf)(A) const) : f(pf) {}
  void operator()(const T& r, A x) const { (r.*f)(x); }
private:
  void (T::*f)(A) const;
};


//通过函数生成以上对应的function object版本
template <class S, class T>
inline mem_fun_t<S,T> mem_fun(S (T::*f)()) { 
  return mem_fun_t<S,T>(f);
}

template <class S, class T>
inline const_mem_fun_t<S,T> mem_fun(S (T::*f)() const) {
  return const_mem_fun_t<S,T>(f);
}

template <class S, class T>
inline mem_fun_ref_t<S,T> mem_fun_ref(S (T::*f)()) { 
  return mem_fun_ref_t<S,T>(f);
}

template <class S, class T>
inline const_mem_fun_ref_t<S,T> mem_fun_ref(S (T::*f)() const) {
  return const_mem_fun_ref_t<S,T>(f);
}

template <class S, class T, class A>
inline mem_fun1_t<S,T,A> mem_fun1(S (T::*f)(A)) { 
  return mem_fun1_t<S,T,A>(f);
}

template <class S, class T, class A>
inline const_mem_fun1_t<S,T,A> mem_fun1(S (T::*f)(A) const) {
  return const_mem_fun1_t<S,T,A>(f);
}

template <class S, class T, class A>
inline mem_fun1_ref_t<S,T,A> mem_fun1_ref(S (T::*f)(A)) { 
  return mem_fun1_ref_t<S,T,A>(f);
}

template <class S, class T, class A>
inline const_mem_fun1_ref_t<S,T,A> mem_fun1_ref(S (T::*f)(A) const) {
  return const_mem_fun1_ref_t<S,T,A>(f);
}

}
#endif
