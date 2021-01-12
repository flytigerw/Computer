


#include "currency.h"
#include "myExceptions.h"

void Currency::setValue(signType sign,unsigned long dollars,unsigned int cents){
        if(cents > 99)
          throw illegalParameterValue("cents must be < 99");
        money = dollars*100+cents;
        if(sign == negative)
          money = -money;
}

void Currency::setValue(double m){
       money = (long)(m*100);
}

Currency Currency::operator+(const Currency& m){
       Currency reuslt;
       reuslt.money = money+m.money;
       return reuslt;
}

Currency& Currency::operator+=(const Currency& m){
       money += m.money;
       return *this;
}

ostream& operator<<(ostream& out,const Currency& c){
       out << "Currency: " << c.getMoney() << "\n"
           << "Dolloars: " << c.getDollars() << "\n"
           << "Cents: " << c.getCents()
           << endl;
       return out;
}
