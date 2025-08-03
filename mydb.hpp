#ifndef _MY_DATABASE_HPP_
#define _MY_DATABASE_HPP_

#include "linkable.hpp"


union DbCell {

};

class DbRow : public Tlinkable {
  DbCell* cells;
};


 class DbTable {
  
};






#endif // _MY_DATABASE_HPP_