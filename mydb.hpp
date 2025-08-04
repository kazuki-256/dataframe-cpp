#ifndef _MY_DATABASE_HPP_
#define _MY_DATABASE_HPP_

#include "linkable.hpp"
#include <vector>
#include <string>




typedef enum DbDataType {
  DB_NULL,
  DB_BOOL,
  DB_INTEGER,
  DB_FLOAT,
  DB_STRING,
  DB_DATE,
} DbDataType;

typedef void* DbPointer;
typedef int64_t DbInt;
typedef double DbFloat;
typedef const char* DbString;
typedef time_t DbDate;



typedef struct DbForeignKeyInfo {
  
} DbForeignKeyInfo;



class DbData {
  union {
    DbPointer asPointer;
    DbInt asInteger;
    DbFloat asNumber;
    DbString asString;
    DbDate asDate;
  } data = 0;
  
  bool isNull = true;
  short extra1 = 0;
  short extra2 = 0;
  
public:
  // 

  DbData& asNull() {
    isNull = true;
    return *this;
  }

  DbData& asBoolean(bool value) {
    isNull = false;
    data.asInteger = value;
    return *this;
  }

  DbData& asPointer(void* pointer) {
    isNull = false;
    data = pointer;
    return *this;
  }

  DbData& asInteger(long integer) {
    isNull = false;
    data = pointer;
    return *this;
  }

  DbData& asFloat() {
    isNull = false;
    data = pointer;
    return *this;
  }

  DbData& asString(const char* str) {
    isNull = false;
    data.asString = str;
    return *this;
  }

  DbData& asDate(time_t date) {
    isNull = false;
    data.asDate = date;
    return *this;
  }
};

class DbRow : public TLinkable {
  std::vector<DbCell> cells;
};


class DbColumn : public TLinkable {
  std::string name;
  DbDateType datetype;

  bool isHidden;
  bool isReadonly;
  bool isForegin;
};


class DbTable {
  TLinkableList<DbColumn> columns;
  TLinkableList<DbRow> rows;
};






#endif // _MY_DATABASE_HPP_