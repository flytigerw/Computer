



#include<iostream> 


//基于数组的树形并查集

int* parent;
//集合父域初始化
void initialize(int n){
  parent = new int[n+1];
  for(int i=1;i<=n;i++)
    parent[i] = 0;
}

int find(int x){
  //追溯到根节点
  while(parent[x] != 0)
    x = parent[x];
  return x;
}

void unite(int rootA,int rootB){
  parent[rootB] = rootA;
}

struct UnionNode{

  bool root;    //表明该节点是否为root
  int parent;   //若为root,则该字段表示以root为根的树的重量 --> 节点数
                //若不为root,则表示父域
  
  UnionNode(){
    parent = 1;root =true;
  }
};

class Union{

  protected:
    UnionNode* nodes;    //数组 

  public:
    Union(int n){
      nodes = new UnionNode[n+1];
    }

  public:
    void unite(int rootA,int rootB){
      //根据重量规则进行合并
      if(nodes[rootA].parent < nodes[rootB].parent){
        nodes[rootB].parent += nodes[rootA].parent;
        nodes[rootA].root = false;
        nodes[rootA].parent = rootB;
      }else{
        nodes[rootA].parent += nodes[rootB].parent;
        nodes[rootB].root = false;
        nodes[rootB].parent = rootA;
      }
    }

    int find(int x){
      //查找时，进行路径紧缩
      //备份
      int y  = x;
      //第一次查找，找到根
      while(!nodes[x].root)
        x = nodes[x].parent;
      int root = x;

      //第二次查找，将路径上的节点的parent修改为root
      int parent;
      while(y != root){
        parent = nodes[y].parent;
        nodes[y].parent = root;
        y = parent;
      }
      return root;
    }
};
