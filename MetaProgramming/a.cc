




#include <iostream>


template<size_t N>
  constexpr bool is_odd = ((N % 2) == 1);

template<size_t N>
struct AllOdd_{
  constexpr static bool is_cur_odd = is_odd<N>;
  constexpr static bool is_pre_odd = AllOdd_<N-1>::value;
  constexpr static bool value = is_cur_odd && is_pre_odd;
};

template<>
struct AllOdd_<0>{
  constexpr static bool value = is_odd<0>;
};


template<typename D>
 struct Base{
    template<typename T>
      void fun(const T& input){
        D* ptr = static_cast<D*>(this);
        ptr->Impl(input);
      }
};

struct Derive : public Base<Derive>{

};









int main(){
  Derive d;
}
  

