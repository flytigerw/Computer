

/* currency: +2.62  -3.11
 * 三个部分: signType:+/-  dollars and cents(0~99)
 * 用更加简洁类对象:long 262,-311 ---> 只要呈现给客户的数据不变即可
 *
 * */

#ifndef CURRENCY
#define CURRENCY 
#include <iostream>
#include <math.h>
using namespace  std;
enum signType{positive,negative};
class Currency{
  friend ostream& operator<<(ostream&,const Currency&);
  public:
    Currency()=default;
    Currency(signType sign,unsigned long dollars,unsigned int cents){ setValue(sign,dollars,cents);}
    Currency(double m){setValue(m);}
    void setValue(signType,unsigned long,unsigned int);      //用户修改类数据的唯一入口，在此处进行参数检查 --> 类的数据就是有效的，后续操作就不需再进行参数检查
    void setValue(double);
    
    double getMoney() const{ return ((double)money)/100; }
    signType getSign() const {return (money<0) ? negative:positive;}
    long getDollars() const {return money/100;}
    int getCents() const {return money%100;}

    //重载+和+=,<<
    //返回右值对象 ----> 临时对象
    Currency  operator+(const Currency&);
    //返回左值对象 ----> 支持流模式
    Currency& operator+=(const Currency&);
  private:
    long money;
};

ostream& operator<<(ostream&,const Currency&);

#endif

