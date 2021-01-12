




#ifndef  COMPLETEWINNERTREE
#define  COMPLETEWINNERTREE 

#include <iostream> 
#include "winnerTree.h"
#include "myExceptions.h"
using namespace std;

template<class T>
class completeWinnerTree : public winnerTree<T>{
  public:
    completeWinnerTree(T* thePlayer,int num) : games(nullptr){
      initialize(thePlayer,num);
    }
    ~completeWinnerTree() override{delete  []games;}
    void initialize(T*, int) override;
    int winner() const override {
      return games[0];
    }
    
    //return winner at gaems[i]
    int winner(int i) const {
      return (i<playersNum) ? games[i] : 0;
    }
    
    void rePlay(int) override;
    void output() const;
  private:
    int lowExt;           //lowest externel nodes number 
    int offset;          //倒数第二层最右节点的编号.
                         //是倒数第一层最左节点相对于根节点的编号偏移
    int* games;         //每一个内部节点代表一场比赛
    int playersNum;
    T* players;
    void play(int,int,int);
};

template<class T>
void completeWinnerTree<T>::initialize(T* thePlayers,int num){
  if(num < 2)
    throw illegalParameterValue("must have at least 2 players");
  players = thePlayers;
  playersNum = num;
  int n = num;
  //销毁以前的比赛数据，重新比赛
  delete []games;
  games = new int[num];

  //计算得到最底层，最左端的内部节点编号 s = 2^log(n-1)-1
  //先算 2^log(n-1)
  int s;
  for(s = 1; 2*s <= n-1; s *= 2);
  --s;
  
  lowExt = 2*(n-s-1);
  offset = 2*(s-1)+2;
  

  //play games for loswest-level externel nodes 
  int i;
  for(i=1; i<lowExt; i += 2)
    play((offset+i+1) / 2,i-1,i);

  //hanle remaining externel nodes 
  if(n %2 == 1){
    //special case for odd n.
    play(n/2-1,games[n-2],lowExt);
    i = lowExt ;
  }else i = lowExt+1;
  
  //i is left-most remaining externel node
}
#endif
