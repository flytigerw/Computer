




#include <iostream>
#include <iterator>
using namespace std;


//f(abcd) = af(bcd)+bf(acd)+cf(bad)
template<class T>
void permutations(T list[],int k,int m){
  if(k == m){
    copy(list,list+m+1,ostream_iterator<T>(cout," "));
    cout << endl;
  }
  
  else{
   for(int i=k;i<=m;i++){
    swap(list[i],list[k]);
    permutations(list,k+1,m);
    swap(list[i],list[k]);
   }
  } 
}
