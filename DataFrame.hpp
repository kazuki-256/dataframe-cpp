#ifndef _MY_DATA_FRAME_HPP_
#define _MY_DATA_FRAME_HPP_

#include <list>
#include <unordered_map>

class DfProcess;



typedef enum DfObjectType {
  DF_OBJTYPE_UNDEFINED,

  DF_OBJTYPE_BOOLEAN,
  DF_OBJTYPE_STRING,
  DF_OBJTYPE_NUMBER,
  DF_OBJTYPE_DATE,
};



class DfObject {
  union {
    void* asPointer;
    bool asBoolean;
    char* asString;
    double asNumber;
    time_t asDate;
  } data;

  uint8_t objType;
  bool    isNull;
};

)
class DfObjectBlock {
  DfObject* array;
  uint32_t lowRange;
  uint32_t highRange;
};


class DfColumn {
  std::list<DfObjectBlock>* blocks;
  std::list<DfColumn*> beForegined;
  DfColumn* foreginTo;

  uint32_t length;
  bool isHidden;
  bool isReadonly;
};


class DfDataFrame {
  std::list<DfColumn> columns;


  DfProcess select(const char* sql);

  DfProcess where(const char* sql);

  DfProcess groupBy(const char* sql);

  DfProcess orderBy(const char* sql);

  DfProcess with(DfDataFrame& other, const char* asName);
};


class DfProcess {
  std::map<std::string
  std::unordered_map<std::string, DfColumn*> columns;


  DfProcess select(const char* sql);

  DfProcess where(const char* sql);

  DfProcess groupBy(const char* sql);

  DfProcess orderBy(const char* sql);

  DfProcess with(DfDataFrame& other, const char* asName);
};



#endif // _MY_DATA_FRAME_HPP_