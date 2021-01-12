

#pragma  once

#include "myExceptions.h"

template<class T>
class WinnerTree{


  public:
    virtual ~WinnerTree(){}

    //初始化:n个选手
    virtual void initialize(T* the_players,int player_num);

    //冠军的索引:哪一个选手
    virtual int winner() const = 0;

    //参赛者分数变化后，进行重赛
    virtual void replay(int playerid,int score) = 0;
};


//用完全二叉树表示
//外部节点为选手，内部为节点为比赛.n个选手会有n-1场比赛
//内部节点从0开始编号，最后一个节点的编号为n-2
//每个内部节点的值表示该场比赛的胜者 ---> 索引选手的编号

template<class T>
class CompleteWinnerTree : public WinnerTree<T>{
  private:
    T* players;
    int* games;
    int players_num;

    //最外层的外部节点数量
    int low_ext;
    //倒数第二层最后一个节点的下标
    int offset;
    
  private:
    void play(int game_idx,int leftChild,int rightChild){
      //小者胜
      games[game_idx] = (players[leftChild] <= players[right]) ? leftChild : rightChild;

      //若game_idx为右节点(偶数下标)，则继续向上比赛
      //若为左节点，则保留比赛数据，供右节点使用
      
      int parent,left,right;
      while(game_idx % 2 == 0 && game_idx>0){
        parent = (game_idx-1) / 2;
        left = games[game_idx-1];
        right = games[game_idx];
        games[parent] = (players[left] <= players[right]) ? games[left] : games[right];
        game_idx = parent;;   //进入上层比赛
      }
    }
    

  public:
    CompleteWinnerTree(T* the_players,int the_num):games(nullptr){
      initialize(the_players,the_num);
    }

    ~CompleteWinnerTree(){
      delete [] games;
    }

  public:
    //总冠军
    int winner() const override {return games[0];}

    //得到指定的第i场比赛的胜利者
    int winner(int i) const {return (i < players_num-1) ? games[i] :0;}


    void initialize(T* the_players,int num) override{
      if(num < 2)
        throw illegalParameterValue("must be at least two players");
      players = the_players;
      players_num = num;

      //销毁以前的比赛数据，重新比赛
      delete []games;
      games = new int[players_num-1];

      //最外层 层首的内部节点的编号 s = 2^log(n-1)-1. 
      //(n-1)个内部节点的层高为:h = log2(n-1)
      int s = 1;
      for(;2*s <= num-1; s*=2);     //非常巧妙

      //最后一层的内部节点数:(n-2)-s+1
      //n-2 --> 最后一个内部节点下标
      //s   --> 行首下标
      //最外层的外部节点数是其二倍
      low_ext = 2*(num-s-1);
      offset = 2*(s-1)+2;


      //先让最外层的外部节点进行比赛
      //比如选手p0和p1，他们的比赛为:g3
      //game = (pi+offset+1)/2-1
      int i;
      for(i=1;i<low_ext;i+=2)
        play((offset+i+1)/2-1,i-1,i);


      int last_game = players_num-2;

      //判断是否有奇数位选手
      if(num % 2 == 1)
        play((last_game-1)/2,games[last_game],--i);

      ++i;
      //让倒数第二层的外部节点进行比赛
      //game = (i-low_ext+1+n-2)/2
      for(;i<num;i+=2)
        play((i-low_ext+1+last_game-1)/2,i,i+1);
    }


    //参赛者分数变化后，进行重赛
    void replay(int i,int score) override{
        if(i < 0 || i >= players_num)
          throw illegalParameterValue("Player index is illegal");
        players[i].score = score;

        int game_idx,left,right;
        int last_game = players_num - 2;
        //针对player所在的位置进行不同的处理
        if(i < low_ext){  //在最外层
          game_idx = (offset+i+1) / 2;
          left = 2*game_idx+1-offset-1;
          right = left+1;
        }else{
          game_idx = (i-low_ext+1+last_game-1)/2;
          //奇数个选手的那个特殊点
          if(2*game_idx + 1 == last_game){
            left = games[last_game];
            right = i;
          }else{
            left = 2*game_idx+1 - last_game + low_ext - 1;
            right = left+1;
          }
        }
        games[game_idx] = (players[left] <= players[right]) ? left : right;

        //奇数个选手的特殊考虑
        if(game_idx == last_game && players_num % 2==1){
          game_idx = (game_idx-1)/2;
          games[game_idx] = (players[games[last_game]] <= players[low_ext]) ?
                      games[last_game] : low_ext;
        }

        //向上追溯到根节点
        while(game_idx>0){
          game_idx = (game_idx - 1) /2;
          left = games[2*game_idx+1];
          right = games[2*game_idx+2];
          games[game_idx] = players[left] <= players[right] ? games[left] : games[right];
        }
    }
  
};

