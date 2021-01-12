
#ifndef HUFFMANNODE
#define HUFFMANNODE


#include "linkedBinTree.h" 

template<class T>
struct huffmanNode{
    operator T() {return weight;}             //以便能够放入小根堆中
    //包括两部分:二叉树+权重
    linkedBinTree<char>* binTree;                //符号为char
    T weight;                                 //权重可以用分数和整数
};
#endif
