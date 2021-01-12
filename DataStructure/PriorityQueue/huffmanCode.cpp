

#include <iostream>
#include "huffmanNode.h"
#include "minHeap.h" 


using namespace std;
//build Huffman tree 
template<class T>
huffmanNode<T>* buildHuffmanTree(T weight [],int n,char chars[]){
  //初始的外部节点集合
  linkedBinTree<char> emptyTree;
  huffmanNode<T>* hNodes = new huffmanNode<T>[n];
  for(int i=0;i<n;i++){
    hNodes[i].weight = weight[i];
    hNodes[i].binTree = new linkedBinTree<char>;
    hNodes[i].binTree.makeTree(chars[i],emptyTree,emptyTree);
  }
  //纳入小根堆
  minHeap<huffmanNode<T>> heap(n);
  heap.initialize(hNodes,n);
  //不断从小根堆中挑选两个weight最小的节点，来构件一个内部节点.一共需要n-1次
  huffmanNode<T> x,y,z;
  for(int i=0;i<n-1;i++){
    x = heap.top(); heap.pop();
    y = heap.top(); heap.pop();

    //build internel node 
    z.weight = x.weight + y.weight;
    z.binTree = new linkedBinTree<T>;
    z.binTree.makeTree(' ',*x.binTree,*y.binTree);

    //再放回小根堆中
    heap.push(z);
  }

}



int main(){

}
