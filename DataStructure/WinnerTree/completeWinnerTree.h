#ifndef COMPLETEWINNERTREE
#define COMPLETEWINNERTREE 

#include <iostream>
#include "winnerTree.h"
#include "myExceptions.h"
using namespace std;


template<class T>
class completeWinnerTree : public winnerTree<T>{
  public:
    //ctor and dctor
    completeWinnerTree(T* thePlayer,int theNum):games(nullptr){
      initialize(thePlayer,theNum);
    }
    ~completeWinnerTree() override {delete  []games;}
    //override functions 
    void initialize(T*,int) override;
    int winner() const override {return games[0];}
    void rePlay(int,int score) override;
    //得到指定的第i场比赛的胜利者
    int winner(int i) const {return (i < playersNum-1) ? games[i] :0;}
    void output() const;
  private:
    int lowExt;          //最底层的外部节点数量
    int offset;         //倒数第二层最后一个节点的编号
    int* games;
    int gamesNum;
    int playersNum;
    T* players;
    void play(int game,int leftChild,int rightChild);
};


template<class T>
void completeWinnerTree<T>::play(int gameIdx,int leftChild,int rightChild){
    
  games[gameIdx] = (players[leftChild] < players[rightChild]) ? leftChild : rightChild;
  //判断gameIdx是否为右节点，若是则继续向上比赛
  int parent;
  int left,right;
  //循环条件:为有节点，不能为根节点
  while(gameIdx % 2 == 0 && gameIdx>0){
    parent = (gameIdx-1) / 2;
    left = gameIdx - 1;
    right = gameIdx;

    games[parent] = (players[left] < players[right]) ? games[left] : games[right];
    gameIdx = parent;;   //进入父节点
  }
}


template<class T>
void  completeWinnerTree<T>::initialize(T* thePlayers,int num){
   

  //check parameter 
  if(num < 2)
    throw illegalParameterValue("must at least two players");
  players = thePlayers;
  playersNum = num;
  gamesNum = num - 1;

  //销毁以前的比赛数据，重新比赛
  delete []games;
  games = new int[gamesNum];
  //compute s = 2^log(n-1)-1. 先计算2^log(n-1)
  int n = num;
  int lastGameIdx = gamesNum - 1;
  int s;
  for(s=1; 2*s <= n-1; s*=2);
  s--;
  
  lowExt = 2*(n-s-1);
  offset = 2*(s-1)+2;

  //先让最底层的外部节点进行比赛
  int i;
  for(i=1;i<lowExt; i+=2)
    play((offset+i+1)/2,i-1,i);     //i为左孩子
  

  //判断是否是奇数位选手
  if(n % 2 == 1){
    play((lastGameIdx-1)/2,games[lastGameIdx],n/2);   
    i++;
  }

  //继续在倒数第二层上比赛
  for(;i<n;i+=2)
    play((i-lowExt+1 + lastGameIdx - 1)/2,i,i+1);
}


template<class T>
void completeWinnerTree<T>::rePlay(int i,int score){
  //check parameter 
  if(i < 0 || i >= playersNum)
    throw illegalParameterValue("Player index is illegal");
  
  players[i].score = score;
  int gameIdx,           //next game is to be played 
      leftChild,rightChild;
  int lastGameIdx = gamesNum - 1;
  
  //find first game and its childern
  if(i < lowExt){    //在最底层
    gameIdx = (offset+i+1) / 2;
    leftChild = 2*gameIdx+1 - offset - 1;
    rightChild = leftChild + 1;
  }else{
    gameIdx = (i-lowExt+1 + lastGameIdx) /2;
    //奇数个选手的那个特殊点
    if(2*gameIdx+1 == lastGameIdx){
      leftChild = games[lastGameIdx];
      rightChild = i;
    }
    else{
      leftChild = 2*gameIdx+1 - (lastGameIdx) + lowExt-1;
      rightChild = leftChild+1;
    }
  }
  games[gameIdx] = (players[leftChild] <= players[rightChild]) ? leftChild : rightChild;

  //special case for second match ---> 奇数场次的问题
  if(gameIdx == lastGameIdx && playersNum % 2 ==1){
    gameIdx = (gameIdx - 1) / 2;
    games[gameIdx] = (players[games[lastGameIdx]] <= players[lowExt]) ?
                      games[lastGameIdx] : lowExt;
  }
  //play remaining games 
  while(true){
     gameIdx = (gameIdx - 1) / 2;
     games[gameIdx] = (players[games[2*gameIdx+1]] <= players[games[2*gameIdx+2]]) ?
      games[2*gameIdx+1] : games[2*gameIdx+2];
     
     if(gameIdx == 0)
       break;
  }
}


template<class T>
void completeWinnerTree<T>::output() const{
  cout << "number of players = " << playersNum 
       << " lowExt = " << lowExt 
       << " offset = " << offset << endl;
  cout << "complte winner tree pointers are" << endl;
  for(int i=0;i<gamesNum;i++)
    cout << games[i] << ' ';
  cout << endl;
  cout << "Players: " << endl;
  for(int i=0;i<playersNum;i++)
    cout << players[i] << ' ';
  cout << endl;
}
#endif


