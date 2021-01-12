#include <iostream>
#include <stack>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <cmath>
using namespace std;

struct pos{
  int x,y;

  pos operator+(const pos& p){
    return {x+p.x,y+p.y};
  }
  
  bool operator<(const pos& p) const {
    return pow(x+3,y) < pow(p.x+3,p.y);
  }
  bool operator == (const pos& p) const {
    return x == p.x && y == p.y;
  }
  
};

//东南西北
pos dirs[4] = {{0,1},{1,0},{0,-1},{-1,0}};

//将数字转为位数之和
int convertNum(int n){
  int sum = 0;
  while(n > 0){
    sum += (n % 10);
    n /= 10;
  }
  return sum;
};

int convertPos(pos p){
  return convertNum(p.x) + convertNum(p.y);
}

bool isValid(const pos& p,int rows,int cols,int k){
        return  p.x >= 0 && p.x<= rows-1 && p.y>=0 && p.y<= cols-1 && convertPos(p) <= k ;
}



int walk(int rows,int cols,int k){
    //参数检查.....
    stack<pos> backPoints;
    map<pos,bool> visited;
    backPoints.push({rows-1,cols-1});
    backPoints.push({0,0});
    pos now = {0,0};
    pos next;
    visited.insert({{0,0},true});
    int amount = 0;
    while(!backPoints.empty() || now == pos({rows-1,cols-1})){
       int i;
       for(i = 0; i<4; i++){
         next = now+dirs[i];
         visited.insert({next,false}); 
         //下一步有效而且没有走过
         if(isValid(next,rows,cols,k) && visited[next] == false){
            ++amount;
            now = next;
            cout << now.x << "-" << now.y << "=" << endl;
            backPoints.push(next);
            visited[now] = true;
            break;
         }
       }
       if(i == 4){ //无路可走，进行回溯
          now = backPoints.top();
          backPoints.pop();
          cout << "回溯点:"<<now.x << "-" << now.y  << endl;
       }
    }
    return amount;
}

int main(){
  cout << walk(12,12,5) << endl;
  return 0;
}
