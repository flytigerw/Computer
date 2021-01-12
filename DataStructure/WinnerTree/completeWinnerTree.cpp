// test winner tree class

#include <iostream>
#include "completeWinnerTree.h"
#include "player.h"

using namespace std;

int main()
{
   int n;
   cout << "Enter number of players, >= 2" << endl;
   cin >> n;
   if (n < 2)
      {cout << "Bad input" << endl;
       exit(1);}


   player *thePlayer = new player[n];

   cout << "Enter player values" << endl;
   for (int i = 0; i < n; i++)
   {
      cin >> thePlayer[i].score;
      thePlayer[i].id = i;
   }

   completeWinnerTree<player> *w = 
      new completeWinnerTree<player>(thePlayer, n);
   cout << "The winner tree is" << endl;
   w->output();

   w->rePlay(1,0);
   cout << "Changed player 1 to zero, new tree is" << endl;
   w->output();

   w->rePlay(3,7);
   cout << "Changed player 3 to 7, new tree is" << endl;
   w->output();

   w->rePlay(4,5);
   cout << "Changed player 4 to 5, new tree is" << endl;
   w->output();

   return 0;
}
