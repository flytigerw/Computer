// test max heap

#include <iostream>
#include "maxHeap.h"

using namespace std;

int main()
{
   // test constructor and push
   maxHeap<int> h(4);
   h.push(10);
   h.push(20);
   h.push(5);

   cout << "Heap size is " << h.size() << endl;
   cout << "Elements in array order are" << endl;
   cout << h << endl;

   h.push(15);
   h.push(30);

   cout << "Heap size is " << h.size() << endl;
   cout << "Elements in array order are" << endl;
   cout << h << endl;

   // test top and pop
   cout << "The max element is " << h.top() << endl;
   h.pop();
   cout << "The max element is " << h.top() << endl;
   h.pop();
   cout << "The max element is " << h.top() << endl;
   h.pop();
   cout << "Heap size is " << h.size() << endl;
   cout << "Elements in array order are" << endl;
   cout << h << endl;
   
   // test initialize
   int z[9];
   for (int i = 0; i < 9; i++)
      z[i] = i;
   h.initialize(z, 9);
   auto x = h;
   cout << "Elements in array order are" << endl;
   cout << x << endl;

   return 0;
}
