
#include <iostream>
#include <vector>
#include <algorithm>

template<typename Function,
         typename SecondArgType>
class bind2nd_impl{

   private:
      Function m_binary_funciton;
      SecondArgType m_second_arg;

   public:
      bind2nd_impl(Function binary_function,SecondArgType second_arg):
          m_binary_funciton(binary_function),
          m_second_arg(second_arg){}

      template<typename FirstArgType>
        auto operator()(FirstArgType&& first_arg) const                          //转为一元谓词
          ->decltype(m_binary_funciton(std::forward<FirstArgType>(first_arg),    //返回类型依赖参数类型.
                                       m_second_arg))                             //所以采用尾置返回类型进行推断
        {
            return m_binary_funciton(std::forward<FirstArgType>(first_arg),
                                     m_second_arg);
        }
};


struct Objects;

void test(){

  std::vector<Objects> objects;

}
