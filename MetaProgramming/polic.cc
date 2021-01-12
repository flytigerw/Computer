






//Policy Group
//Major Class
struct Accpolicy{
  //Minor Class1
  struct AccuTypeCate{
      struct Add;
      struct Mul;
  };
  //default policy of Minor Class1 
  using Accu = AccuTypeCate::Add;

  struct IsAveValueCate;
  static constexpr bool IsAve = false;

  struct ValueTypeCate;
  using Value = float;

};


struct PMulAccu : virtual public Accpolicy{
    //Identifier
    using MajorClass = Accpolicy;
    using MinorClass = Accpolicy::AccuTypeCate;
    //Value 
    using Accu = AccuTypeCate::Mul;       //改变了默认的policy值
};
