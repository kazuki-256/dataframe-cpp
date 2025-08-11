#ifndef _MY_DATA_FRAME_HPP_
#define _MY_DATA_FRAME_HPP_

#include <list>



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
};




#endif // _MY_DATA_FRAME_HPP_