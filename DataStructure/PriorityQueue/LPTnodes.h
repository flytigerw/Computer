
#ifndef LPTNODES
#define LPTNODES 


class JobNode{
  friend void makeSchedule(JobNode*,int,int);
  friend int main();
  public:
    operator int() const {return time;} //使其能够在小根堆中转换为int后进行比较，然后排序
  private:
    int id,      //jod identifier
        time;
};

class MachineNode{
  friend void makeSchedule(JobNode*,int,int);
  public:
   //ctor 
   MachineNode(int theID=0,int theAvail = 0) : id(theID),avail(theAvail){}

   operator int() {return avail;} //道理同上
  private:
  int id;           //machine identifier
  int avail;       //when it becomes free
};

#endif 
