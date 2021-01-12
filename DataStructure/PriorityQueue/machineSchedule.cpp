#include <iostream>
#include "LPTnodes.h"
#include "maxHeap.h"
#include "minHeap.h"
using namespace  std;


void makeSchedule(JobNode a[],int n,int m){
  if(n <= m){
    cout << "Schedule each job on a different Machine" << endl;
    return;
  }
  //对作业排序
  heapSort(a,n);

  //initialize m Machines and the min heap 
  minHeap<MachineNode> MachineHeap(m);
  for(int i=1;i<=m;i++)
    MachineHeap.push(MachineNode(i,0));

  //construct schedule 
  MachineNode machine;
  for(int i=n-1;i>=0;i--){
      machine = MachineHeap.top();
      MachineHeap.pop();
      cout << "Schdule Job" << a[i].id << "on Machine" << machine.id << " from " << machine.avail;
      machine.avail += a[i].time;
      MachineHeap.push(machine);
  }
}

int main(){
  return 0;
}
