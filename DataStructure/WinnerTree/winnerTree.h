

#ifndef WINNERTREE
#define WINNERTREE 

template<class T>
class winnerTree{
  public:
    virtual ~winnerTree(){}
    virtual void initialize(T* thePlayers,int playerNum) = 0;
    //返回冠军的索引
    virtual int winner() const  = 0;
    virtual void rePlay(int playerId,int score) = 0;
};


#endif
