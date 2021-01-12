// node type used by winner tree
#ifndef player_
#define player_

struct player
{
   int id, score;
   operator int () const {return score;}
};

#endif
