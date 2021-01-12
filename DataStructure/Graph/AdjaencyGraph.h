

#pragma once
#include <list>
#include "Graph.h"
#include "myExceptions.h"
#include "myArray.h"

//基于邻接矩阵的图

//加权有向图

//T为权的类型
template<class T>
class AdjacenyWDGraph : public Graph<T>{
  protected:
    int n;        //顶点数
    int e;        //边数
    T** matrix;        //邻接矩阵
    T no_edge;    //在矩阵中表示该边不存在
  
    //迭代器设计
    //如何定位图中的某个点
    //
  public:
    class MyIterator : public VertexIterator<T>{



      protected:
        T* row;   //定位到行
        int cur_col;  //列号

        //辅助信息
        T no_edge;


      public:
        //定位到第一个 邻接于 当前顶点的点
        int next() override{
          for(int i=cur_col;i<=n;i++)
            if(row[i] != no_edge){
              cur_col = i+1;
              return i;
            }

          

        }



    };



  protected:
    void check_vertex(int v) const{
      if(v < 1 || v > n){
        ostringstream s;
        s << "no vertex" << v;
        throw illegalParameterValue(s.str());
      }
    }



  public:
    
    AdjacenyWDGraph(int numberOfVertices,T the_no_edge){
         if (numberOfVertices < 0)
            throw illegalParameterValue("number of vertices must be >= 0");
         n = numberOfVertices;
         e = 0;
         no_edge = the_no_edge;

         //构建(n+1)*(n+1)的二维数组
         make2dArray(matrix,n+1,n+1);
         //初始化为no_edge
         for(int i=1;i<=n;i++)
           fill(matrix[i],matrix[i]+n+1,no_edge);
    }

    ~AdjacenyWDGraph(){
      delete2dArray(matrix,n+1);
    }
    //顶点数
    int number_of_vertices() const override{
      return n;
    }
    //边数
    int number_of_edges() const override{
      return e;
    }
    //边(i,j)是否存在
    bool exists_edge(int i, int j) const override{
      //边界检查
      if(i < 1 || j < 1 || i>n || j>n)
        return false;
      if(matrix[i][j] == no_edge)
        return false;
      return true;
    }
    //插入新边
    void insert_edge(Edge<T>* edge) override{
      int v1=edge->vertex1();
      int v2=edge->vertex2();
      //边界检查
      if (v1 < 1 || v2 < 1 || v1 > n || v2 > n || v1 == v2){
            ostringstream s;
            s << "(" << v1 << "," << v2 
              << ") is not a permissible edge";
            throw illegalParameterValue(s.str());
         }

      //新边
      if(matrix[v1][v2] == no_edge)
        ++e;
      //权重更新
      matrix[v1][v2] = edge->weight();
    }
    //删除边
    void erase_edge(int i, int j){
      //不做错误处理
      //边界检查.边是否存在检查
      if(i >= 1 && j>=1 && i<=n && j<=n && matrix[i][j] != no_edge){
        matrix[i][j] = no_edge;
        --e;
      }

    }
    //顶点的度:无定义
    int degree(int) const override{
       throw undefinedMethod("degree() undefined");
    }

    //出度
    //固定行，遍历列
    int out_degree(int v) const override{
      check_vertex(v);
      int sum = 0;
      for(int i=1;i<=n;i++)
        if(matrix[v][i] != no_edge)
          ++sum;
      return sum;
    }
    //入度
    //固定列，遍历行
    int in_degree(int v) const override{
      check_vertex(v);
      int sum = 0;
      for(int i=1;i<=n;i++)
        if(matrix[i][v] != no_edge)
          ++sum;
      return sum;
    }
    //是否有向边
    bool directed() const override{
      return true;
    }
    //是否为加权边
    bool weighted() const override{
      return true;
    }

    //获取顶点i的迭代器
    virtual VertexIterator<T>* iterator(int) = 0;


    //dist存储从source到各个顶点的最短路径值
    //prevs存储从source到各个顶点的最短路径 --> 存储前驱
    void dijkstra(int source,T* dist,int* prevs){
      //check
      if(source < 1 || source > n)
        throw illegalParameterValue("Invalid source vertex");
      if(!weighted())
        throw undefinedMethod("adjacencyWDigraph::shortestPaths() not defined for unweighted graphs");

      std::list<int> v2;        //下一步可选择的顶点

      //初始化
      for(int i=1;i<=n;i++){
        dist[i] = matrix[source][i];
        if(dist[i] == no_edge)
          prevs[i] = -1;       
        else{
          prevs[i] = source;
          v2.push_back(i);
        }
      }
      //不停地从v2中选择出顶点min_vertext,构成最短边
      //将min_vertext加入到v1中
      //更新dist和v2
      while(!v2.empty()){
          auto min_vertex_iter = v2.begin();
          int min_vertex;
          //现在v2中找到min_vertex
          for(auto it=v2.begin();it != v2.end(); ++it)
            if(dist[*min_vertex_iter] > dist[*it])
              min_vertex_iter = it;
          min_vertex = *min_vertex_iter;
          v2.erase(min_vertex_iter);

          //更新dist 
          for(int i=1;i<=n;i++){
            if((prevs[i] == -1 && matrix[min_vertex_iter][i] != no_edge)   //之前不可达
               || (dist[i] > dist[min_vertex_iter]+matrix[min_vertex][i]))    //之前路径过长
            {
                dist[i] = dist[min_vertex]+matrix[min_vertex][i];
                //更新v2
                if(prevs[i] == -1)
                  v2.push_back(i);

                prevs[i] = min_vertex;
            }
          }
      }
    }

    void prim(WeightedEdge<T> mst[]){
      //no check
      
      int N = number_of_vertices();
      //初始化mst.以顶点1开始
      for(int i=1;i<N;i++){
        mst[i].m_v1 = 1;
        mst[i].m_v2 = i+1;
        mst[i].m_weight = matrix[1][i+1];
      }

      T weight;
      int min_idx,y,dest;
      //逐个挑选n-1条最短边
      for(int i=1;i<N;i++){

        //从mst[i]到mst[N-1]中选出最短边(x,y)
        for(int j=i;j<N;j++)
          if(mst[j].m_weight < weight){
            weight = mst[j].m_weight;
            min_idx = j;
          }

        //将点y加入到V1中 <---> 将边(x,y)放到对应位置上
        y = mst[min_idx].m_v2;
        swap(mst[i],mst[min_idx]);

        //更新mst ---> y的加入可能使得V1和V2之间的短边更短
        for(int j=i+i;j<N;j++){
          dest = mst[j].m_v2;
          if(mst[j].m_weight > matrix[y][dest]){
            //更新
            mst[j].m_v1 = y;
            mst[j].m_weight = matrix[y][dest];
          }
        }
      }
    }
};

