

#pragma once

#include "dictioary.h"
#include "myExceptions.h"
#include <sstream>
#include <math.h>

//在跳表中存储键值对
//节点以K来排序
template<class K,class V>
struct SkipNode{
  typedef std::pair<const K,V> KV;
  KV m_kv;
  //跳表节点的next具有层级属性 
  SkipNode** next;  //数组 
  SkipNode(const KV& kv,int size):m_kv(kv){
    next = new SkipNode*[size];
  }
  ~SkipNode(){
    delete []next;
  }
};


template<class K,class V>
class SkipList : public dictionary<K,V>{

  
  public:
    typedef std::pair<const K,V> KV;
    typedef SkipNode<K,V> Node;

  protected:
    //头尾Node
    Node* head,*tail;

    //当前达到的最大层级
    int cur_max_level =0;

    //跳表允许的最大层级 
    int max_level;

    //跳表运行的最大key
    K largest_key;

    //存储的键值对个数
    int m_size =0;

    //插入与删除时，若节点位于第i层，则该节点0~i层的前驱指针都需要改变.
    //也可以插入和删除中将其定义为临时变量 ---> 但需要不停地申请和释放 ---> 故将其作为成员变量
    Node** pres;      //数组

    
    //在规则的跳表中,上层与下层的节点个数比: p = 0.5 
    //当然，用户也可自行决定
    int m_probablity;
    

    //私有辅助函数
  protected:
    //通过随机数产生器来决定新节点的level
    int level() const;

    //遍历搜集前驱节点
    Node* search(const K&) const; 
  
  public:
    SkipList(K, int max_pairs = 10000, float prob = 0.5);
    ~SkipList();

    //对外接口
  public:
    bool empty() const override{return m_size == 0;}
    int size()const override{return m_size;}
    KV* find(const K& ) const override;
    void insert(const KV&) override;
    void erase(const K&) override;
    void output(std::ostream& out) const;
};

template<class K,class V>
SkipList<K,V>::SkipList(K key,int max_pairs,float p):largest_key(key),m_probablity(p){

  //最大节点数为n,max_level = ceil(log(1/p)(n))- 1
  max_level = (int)(ceil(logf((float)max_pairs)) / logf(1/p))-1;


  KV largest_kv;
  largest_kv.first = largest_key;

  //层级和size之间关系: size = level+1
  head = new Node(largest_kv,max_level+1); 
  
  tail = new Node(largest_kv,0);

  //初始时，head的每一层的next都指向tail
  for(int i=0;i<=max_level;i++)
    head->next[i] = tail;

  pres = new Node*[max_level+1];
}


template<class K,class V>
SkipList<K,V>::~SkipList(){
    //类似单向链表的删除
    Node* node;
    while(head != tail){
      //先保存下一个节点，再删除当前节点
      node = head->next[0];
      delete node;
      head = node;
    }

    delete tail;
    delete []pres;
}

//找到该节点，并收集该节点在历来层级上的前驱
template<class K,class V>
typename SkipList<K,V>::Node*
SkipList<K,V>::search(const K& key) const{

  //从最大层开始向下找
  Node* node = head;
  for(int i=cur_max_level;i>=0;i--){
    //node->next[i]:二分点
    //若二分点的key较小，则向起点向右挪动
    while(node->next[i].m_kv.first < key)
      node = node->next[i];

    //若二分点的key较大，则起点不变，只降低层级即可
    
    //记录前驱
    pres[i] = node;
  }
  return node->next[0];
}

template<class K,class V>
typename SkipList<K,V>::KV*
SkipList<K,V>::find(const K& key) const{


  if(key >= largest_key)
    return nullptr;

  //类似search,但不需要保存前驱
  Node* node = head;
  for(int i=cur_max_level;i>=0;i--)
    while(node->next[i].m_kv.first < key)
      node = node->next[i];
  //若二分点的key较大，则起点不变，只降低层级即可
  
  //若找到了就返回
  if(node->next[0]->m_kv.fisr == key)
    return &node->next[0]->m_kv;

  return nullptr;
}

template<class K,class V>
int SkipList<K,V>::level() const{

  //生成随机数范围 0~RAND_MAX 
  //若<= p*RADN_MAX，就++level
  
  int lev = 0;
  while(rand() <= m_probablity*RAND_MAX)
    ++lev;

  //不能超过最大层级
  return (lev<=max_level) ? lev : max_level;
}

template<class K,class V>
void
SkipList<K,V>::insert(const KV& kv) {


  if(kv.first >= largest_key){
    std::ostringstream s;
    s << "Key = " << kv.first << " Must be <" << largest_key;
    throw illegalParameterValue(s.str());
  }

  //先查找其是否存在，以及前驱节点
  Node* node = search(kv.first);
  if(node->m_kv.first == kv.first){  //存在，就只更新V
    node->m_kv.second = kv.second;
    return;
  }

  //没找到，就新建节点
  int new_level = level();
  //下调
  if(new_level > cur_max_level){
    new_level = ++cur_max_level;
    pres[new_level] = head;  //可要，可不要
  }

  //创建新节点
  Node* new_node = new Node(kv,new_node+1);
  //改变下层链表的前驱节点指针
  for(int i=0;i<=new_level;i++){
    //插入到中间
    new_node->next[i] = pres->next[i];
    pres->next[i] = new_node;
  }

  ++m_size;

  return;
}

template<class K,class V>
void
SkipList<K,V>::erase(const K& key) {
  if(key >= largest_key){
    std::ostringstream s;
    s << "Key = " << key << " Must be <" << largest_key;
    throw illegalParameterValue(s.str());
  }

  //先查找该节点
  Node* node = search(key);
  //考察是否找到
  if(node->m_kv.first != key)
    return;

  //自底向上的删除
  //SkipNode没有指明自己的level有多高
  for(int i=0;i<=cur_max_level && pres[i]->next == node;i++){
    //和普通链表的操作是一样的
    pres->next[i] = node->next[i];
  }

  //删除节点可能导致最高层已经没有节点，head指向tail
  //此时需要降级
  while(cur_max_level > 0 && head[cur_max_level] == tail)
    --cur_max_level;
  delete node;
  --m_size;
}

template<class K, class V>
void SkipList<K,V>::output(ostream& out) const{
   // Insert the dictionary pairs into the stream out.
   // follow level 0 chain
   for (Node* currentNode = head->next[0];
                       currentNode != tail;
                       currentNode = currentNode->next[0])
      out << currentNode->element.first << " "
          << currentNode->element.second << "  ";
}

// overload <<
template <class K, class E>
ostream& operator<<(ostream& out, const SkipList<K,E>& x){
     x.output(out); 
     return out;
}
