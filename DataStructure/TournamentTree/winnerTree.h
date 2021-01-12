


#ifndef WINNERTREE
#define WINNERTREE 

template<class T>
class winnerTree{
  public:
    virtual ~winnerTree(){}
    virtual void initialize(T* thePlayers,int theNum)=0;
    virtual int  winner() const = 0;
    virtual void rePlay(int thePlayer) = 0;
};
#endif
