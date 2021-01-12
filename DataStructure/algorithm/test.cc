 
#include <vector>
#include<stack>
#include <string.h>
using namespace std;

class Solution {
public:
    
    struct Pos{
      int x,y;
      int c = -1;                 //已经走过的方向
      Pos(int i,int j):x(i),y(j){}
    };
    
    using V = vector<int>;
    using VV = vector<V>;
    
   bool out(const Pos& p,int rows,int cols){
        return p.x<0 || p.x>rows-1 || p.y<0 || p.y>cols-1;
    }
    
    bool illegal(const Pos& p,const VV& v){
        return v[p.x][p.y] == 1;
    }
    
    bool hasPath(const char* matrix, int rows, int cols, const char* str)
    {
        VV v(rows,V(cols,0));
        int len = strlen(str);
        int dirs[4][2]  = {{0,1},{1,0},{0,-1},{-1,0}}; //东南西北
        stack<Pos> s;
        vector<Pos> starts; 
        for(int i=0;i<rows*cols-1;i++){
            if(matrix[i] == str[0])
                starts.push_back(Pos(i/cols,i%cols));
        }
        if(starts.empty())
            return false;
        
        for(const auto& start:starts){
            //每个起点的初始化
            s.push(start);
            v[start.x][start.y] = 1;
            int idx = 1;
            while(!s.empty()){
                auto& t = s.top();
                int i = 0;
                for(;i<4;i++){
                    if(t.c >= i)
                        continue;
                    t.c = i;
                    auto dir = dirs[i];
                    Pos new_pos = Pos(t.x+dir[0],t.y+dir[1]);
                    if(out(new_pos,rows,cols) || illegal(new_pos,v)) 
                       continue;
                    ///成功匹配到一个字符,然后break
                    if(matrix[new_pos.x*cols+new_pos.y] == str[idx]){
                        s.push(new_pos);
                        v[new_pos.x][new_pos.y] = 1;
                        if(++idx == len)
                           return true;
                        break;
                    }
                }
                //四个方向匹配失败 ---> 回溯
                if(i == 4){
                    v[t.x][t.y] = 0;
                    s.pop();
                    --idx;
                }
            }
        }
        return false;
    }


};

int main(){
  Solution s;
  s.hasPath("ABCEHJIGSFCSLOPQADEEMNOEADIDEJFMVCEIFGGS",5,8,"SLHECCEIDEJFGGFIE");
}
