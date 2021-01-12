#pragma once 

#include <iostream>
#include "myExceptions.h"
#include "../PriorityQueue/minHeap.h"
#include "../PriorityQueue/binTreeNode.h"
#include "../StackAndQueue/ArrayQueue.h"
#include "../StackAndQueue/ArrayStack.h"
#include "Union.h"
#include <queue>


//边的操作接口 
//获取其关联的顶点+权值
//顶点采用整数描述
template<class T>    //可能为加权边
class Edge{
  public:
    virtual ~Edge(){}
    virtual int vertex1() const = 0;
    virtual int vertex2() const = 0;
    virtual T weight() const = 0;
};

//点迭代器操作接口
template<class T>
class VertexIterator{
  public:
    virtual ~VertexIterator(){}
    //相邻的顶点中选一个
    virtual int next() = 0;
    virtual int next(T& ) = 0;  //与next顶点构成的边的权值
};

//带有权重的边
template<class T>
class WeightedEdge : public Edge<T>{
  private:
    int m_v1,m_v2;
    T m_weight;
  public:
    WeightedEdge(){}
    WeightedEdge(int v1,int v2,T weight):m_v1(v1),m_v2(v2),m_weight(weight){}
    ~WeightedEdge(){}

  public:
    int vertex1() const override{return m_v1;}
    int vertex2() const override{return m_v2;}

    T weight() const override {return m_weight;}

    //可以直接转为权值
    operator T() const {return m_weight;}

    void output(std::ostream& out) const{
      out << "(" << m_v1 << "," << m_v2 <<","  << m_weight << ")";
    }
};

template <class T>
ostream& operator<<(ostream& out, const WeightedEdge<T>& x){
  x.output(out); return out;
}

template<class T>
class Graph{

  //操作接口
  public:
    virtual ~Graph() {}
    //顶点数
    virtual int number_of_vertices() const = 0;
    //边数
    virtual int number_of_edges() const = 0;
    //某条边是否存在
    virtual bool exists_edge(int, int) const = 0;
    //插入新边
    virtual void insert_edge(Edge<T>*) = 0;
    //删除边
    virtual void erase_edge(int, int) = 0;
    //顶点的度
    virtual int degree(int) const = 0;
    //入度
    virtual int in_degree(int) const = 0;
    //出度
    virtual int out_degree(int) const = 0;
    //是否有向边
    virtual bool directed() const = 0;
    //是否为加权边
    virtual bool weighted() const = 0;
    //获取顶点i的迭代器
    virtual VertexIterator<T>* iterator(int) = 0;
    virtual void output(ostream&) const = 0;



  protected:
    //静态变量，与对象无关 --> 适用于所有类型的子类对象
    

    static int* reach;    //用于在深度优先递归遍历时表示已路过的顶点:数组下标表示顶点编号.数组值为label,表示该点以达到
    static int lable;

    static int* path;    //存储路径搜寻时的路径:点集
    static int length;   //路径长度
    static int destination; //路径终点


    //图的一些操作
  public:

    //广度优先遍历
    //v为起始顶点
    virtual void bfs(int v,int reach[],int label){
      ArrayQueue<T> q(10);
      //初始状态
      q.push(v);
      reach[v] = label;
      int cur,next;
      while(!q.empty()){
        cur = q.front();
        q.pop();

        //压入所有的 与其邻接 但没标记的顶点，并打上标记
        //获取当前顶点的迭代器，调用next()获取邻接顶点
        VertexIterator<T>* it = iterator(cur);
        while((next = it->next()) != 0)
          if(reach[next] == 0){
            q.push(next);
            reach[next] = label;
          }

        delete it;
      }
    }

    void dfs(int v,int reach[],int label){
      //静态变量的初始化
      Graph<T>::reach = reach;
      Graph<T>::lable = label;
      dfs_impl(v);
    }

    void dfs_impl(int v){
      //label v as reached 
      reach[v] = lable;
    
      //还是通过迭代器获取邻接顶点
      VertexIterator<T>* it = iterator(v);
      int next;
      //针对每一个 邻接的 未到达的顶点，进行递归深入
      while((next = it->next()) != 0)
        if(reach[next] == 0)
          dfs_impl(next);
      delete  it;
    }


    //寻找从source到destination的路径
    //采用深度优先
    int* find_path(int source,int destination){

      //静态变量初始化
      int n = number_of_vertices();
      path = new int[n+1];
      path[1] = source;
      length = 1; 

      //开始路径搜寻
      if(source == destination || find_path_impl(source))
        path[0] = length-1;  //找到路径，将路径长度存储在path[0]中
      else{
        delete []path;
        path = NULL;
      }
      delete []reach;
      return path;
    }

    bool find_path_impl(int source){

      //采用深度优先
      reach[source] = 1;
      VertexIterator<T>* it = iterator(source);
      int next;
      while((next = it->next()) != 0){
        if(reach[next] == 0){
          //路径存入
          path[++length] = next;

          //递归深入
          if(next == destination || find_path_impl(next))
            return true;
          //回溯
          --length;   //可覆盖 = 舍弃
        }
      }
      delete it;

      return false;
    }

    //判断无限
    bool connected(){
      //不适用于有向图
      if(directed())
         throw undefinedMethod("graph::connected() not defined for directed graphs");
      
      //静态变量初始化
      int n = number_of_vertices();
      reach = new int[n+1];

      //从顶点1开始进行深度优先遍历
      dfs(1,reach,1);

      //检查是否遍历到了所有的顶点
      //若所有顶点都被遍历到，则说明所有顶点之间存在一条路径
      for(int i=1;i<=n;i++)
        if(reach[i] == 0)
          return false;
      return true;
    }

    bool kruskal(WeightedEdge<T>* mst){
         if (directed() || !weighted())
            throw undefinedMethod
            ("graph::kruskal() not defined for unweighted and directed graphs");


         int N = number_of_vertices();
         int E = number_of_edges();

         std::priority_queue<WeightedEdge<T>,
                             vector<WeightedEdge<T>>,
                             less<WeightedEdge<T>>> heap;

         //并查集，用于检测环路
         Union u(N);
         int a,b;
          //使用小根堆存储所有的边 ---> E2
         int i =0;
         while(E > 0 && i < N-1){
           //从E2中选出最短边(x,y)
           auto e = heap.top();
           heap.pop();
           E--;

           //检验边是否会形成环路
           //看两点是否位于同一集合中
           a = u.find(e.vertex1());
           b = u.find(e.vertex2());
           if(a != b){
             mst[i++] = e;
             u.unite(a,b);
           }
         }
         return i == N-1;
    }

};






