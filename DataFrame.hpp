#ifndef _MY_DATA_FRAME_HPP_
#define _MY_DATA_FRAME_HPP_

#include <list>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <string>

#include <time.h>
#include <string.h>
#include <stdarg.h>

class DfObject;
class DfObjectBlock;
class DfColumn;
class DfDataFrame;
class DfRow;
class DfProcess;



typedef enum DfObjectType {
  DF_OBJTYPE_UNDEFINED,

  DF_OBJTYPE_POINTER,
  DF_OBJTYPE_BOOLEAN,
  DF_OBJTYPE_STRING,
  DF_OBJTYPE_NUMBER,
  DF_OBJTYPE_DATE,
};


class DfException : public std::exception {
  char* msg;
public:
  DfException(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t len = strlen(fmt);
    msg = (char*)malloc(len + 378);
    vsnprintf(msg, len + 377, fmt, args);

    va_end(args);
  }

  ~DfException() {
    free(msg);
  }

  const char* what() const noexcept override {
    return msg;
  }
};


class DfExceptionOutOfIndex : public std::exception {
  const char* what() const noexcept override {
    return "out of index";
  }
};




class DfDate {
  time_t t;
  static char sharedBuffer[64];

public:
  DfDate(time_t _t) {
    t = _t;
  }

  DfDate(const char* strdate) {
    
  }


  const char* toString(char* buf = sharedBuffer, const char* fmt = "%d/%m/%y %H:%M") const {
    
  }
};


class DfObject {
  union {
    void* asPointer;
    bool asBoolean;
    char* asString;
    double asNumber;
    time_t asDate;
  } data;

  int extra;   // for string
  uint8_t objType;
  bool    null;


  inline void init(uint8_t _objType, bool _null) {
    objType = _objType;
    null = _null;
  }

  void copyFrom(const DfObject& src) {
    init(src.objType, src.null);

    if (isNull()) {
      return;
    }

    if (objType == DF_OBJTYPE_STRING) {
      extra = src.extra;
      data.asString = (char*)malloc(extra + 1);
      strcpy(data.asString, src.data.asString);
      return;
    }

    data = src.data;
  }

public:
  ~DfObject() {
    if (objType == DF_OBJTYPE_STRING) {
      free(data.asString);
    }
  }

  // create empty object
  DfObject(DfObjectType _objType = DF_OBJTYPE_UNDEFINED) {
    init(_objType, true);
    data.asPointer = NULL;
  }

  // set pointer
  DfObject(void* pointer) {
    init(DF_OBJTYPE_POINTER, false);
    data.asPointer = pointer;
  }

  // set boolean
  DfObject(bool boolean) {
    init(DF_OBJTYPE_BOOLEAN, false);
    data.asBoolean = boolean;
  }

  // set string
  DfObject(const std::string str) {
    init(DF_OBJTYPE_STRING, false);

    extra = (int)str.length();
    data.asString = (char*)malloc(extra + 1);
    strcpy(data.asString, str.c_str());
  }

  // set number
  DfObject(double number) {
    init(DF_OBJTYPE_NUMBER, false);
    data.asNumber = number;
  }

  // set number
  DfObject(long number) {
    init(DF_OBJTYPE_NUMBER, false);
    data.asNumber = (double)number;
  }

  // set number
  DfObject(int number) {
    init(DF_OBJTYPE_NUMBER, false);
    data.asNumber = (double)number;
  }

  // set date
  // DfObject(time_t date) {
  //   init(DF_OBJTYPE_DATE, false);
  //   data.asDate = date;
  // }



  DfObjectType getObjectType() const {
    return (DfObjectType)objType;
  }

  bool isNull() const {
    return null;
  }



  // get pointer
  operator void*() const {
    if (objType != DF_OBJTYPE_POINTER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asPointer;
  }

  // get boolean
  operator bool() const {
    if (objType != DF_OBJTYPE_BOOLEAN && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asBoolean;
  }

  // get string
  operator const char*() const {
    if (objType != DF_OBJTYPE_STRING && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asString;
  }

  // get number
  operator double() const {
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asNumber;
  }

  // get number
  operator long() const {
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return (long)data.asNumber;
  }

  // get number
  operator int() const {
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return (int)data.asNumber;
  }

  // get date
  // operator time_t() const {
  //   if (objType != DF_OBJTYPE_DATE && objType != DF_OBJTYPE_UNDEFINED) {
  //     throw DfException("couldn't get data by different type!");
  //   }
  //   return data.asDate;
  // }


  
  // move
  DfObject(DfObject&& src) {
    init(src.objType, src.null);

    if (isNull()) {
      return;
    }

    if (objType == DF_OBJTYPE_STRING) {
      extra = src.extra;
      data.asString = src.data.asString;
      src.data.asString = NULL;
      return;
    }

    data = src.data;
    return;
  }
  
  
  // copy1
  DfObject(const DfObject& src) {
    copyFrom(src);
  }

  // copy2
  inline DfObject& operator=(const DfObject& src) {
    copyFrom(src);
    return *this;
  }
};



class DfObjectBlock {
  friend class DfColumn;

  DfObject* objects;
  int lowRange;
  int highRange;



  DfObjectBlock(DfObjectType objType, int _lowRange, int _highRange) {
    int length = _highRange - _lowRange + 1;

    objects = new DfObject[length];
    lowRange = _lowRange;
    highRange = _highRange;

    // -- fastest array copies --
    DfObject* out = objects;
    DfObject* end = objects + length;

    while (out < end) {
      *(out++) = DfObject(objType);
    }
  }

  // copy for other array
  DfObjectBlock(const DfObject* _objects, int _length, int _lowRange, int _highRange) {
    objects = new DfObject[_length * sizeof(DfObject)];
    lowRange = _lowRange;
    highRange = _highRange;

    // -- fastest array copies --
    DfObject* out = objects;
    const DfObject* in  = _objects;
    DfObject* end = objects + _length;

    while (out < end) {
      *out = *in;
      out++, in++;
    }
  }

  DfObjectBlock(const std::initializer_list<DfObject>& _objects, int _lowRange, int _highRange) {
    objects = new DfObject[_objects.size() * sizeof(DfObject)];
    lowRange = _lowRange;
    highRange = _highRange;

    int index = 0;
    for (const DfObject& obj : _objects) {
      objects[index++] = obj;
    }
  }


  // return a editable object, use real position
  DfObject& operator[](int index) {
    return *(objects + index - lowRange);
  }

public:
  ~DfObjectBlock() {
    if (objects == NULL) return;
    
    delete[] objects;
  }
};



class DfColumn {
  std::list<DfObjectBlock>* blocks;
  std::list<DfColumn*> beForegined;
  DfColumn* foreginTo;

  int length;
  uint8_t objType;
  bool isHidden;
  bool isReadonly;      // not usable currectly
  bool isOwnsBlocks;

  
  inline void init(bool _isReadOnly) {
    isHidden = false;
    isReadonly = _isReadOnly;
    isOwnsBlocks = true;
    blocks = new std::list<DfObjectBlock>;
  }
  
  DfObject& getObjectAt(int index) const {
    // -- negivate find --
    if (index < 0) {
      index = length + index;

      if (index < 0 && index >= length) {
        throw DfExceptionOutOfIndex();
      }

      for (auto iter = blocks->rbegin(); iter != blocks->rend(); iter++) {
        if (iter->lowRange >= index) {
          return (*iter)[index];
        }
      }
      throw DfException("ObjectBlock didn't seted");
    }
    
    // -- positive find --
    if (index >= length) {
      throw DfExceptionOutOfIndex();
    }

    for (DfObjectBlock& block : *blocks) {
      if (index <= block.highRange) {
        return block[index];
      }
    }
    throw DfException("ObjectBlock didn't seted");
  }

public:

  ~DfColumn() {
    if (isOwnsBlocks) {
      delete blocks;
    }
  }

  DfColumn(const std::initializer_list<DfObject>& _objects, bool _isReadonly = false) {
    init(isReadonly);

    length = _objects.size();
    if (length == 0) {
      objType = DF_OBJTYPE_UNDEFINED;
    }
    else {
      objType = _objects.begin()->getObjectType();
    }

    blocks->push_back(DfObjectBlock(_objects, 0, length - 1));
  }

  DfColumn(DfObjectType _objType = DF_OBJTYPE_UNDEFINED, bool _isReadOnly = false) {
    init(isReadonly);

    length = 0;
    objType = _objType;
  }


  
  DfColumn& addObject(const DfObject& object) {
    if (objType != object.getObjectType()) {
      if (objType != DF_OBJTYPE_UNDEFINED) {
        throw DfException("couldn't add %d type object to %d type column!", object.getObjectType(), objType);
      }
      objType = object.getObjectType();
    }

    DfObjectBlock& last = *blocks->end();
    if (last.highRange <= length) {
      last[length] = object;
      return *this;
    }

    blocks->push_back(DfObjectBlock(&object, 0, length, length + 7));
    return *this;
  }

  DfColumn& addObject(DfObject&& object) {
    if (objType != object.getObjectType()) {
      if (objType != DF_OBJTYPE_UNDEFINED) {
        throw DfException("couldn't add %d type object to %d type column!", object.getObjectType(), objType);
      }
      objType = object.getObjectType();
    }

    DfObjectBlock& last = *blocks->end();
    if (last.highRange <= length) {
      last[length] = object;
      return *this;
    }

    blocks->push_back(DfObjectBlock(getObjectType(), length, length + 7));
    blocks->end()->objects[0] = object;
    return *this;
  }



  const DfObject& operator[](int index) const {
    return getObjectAt(index);
  }

  DfObject& operator[](int index) {
    if (index >= length) {
      addObject(DfObject(getObjectType()));
      return (*blocks->end())[length];
    }
    return getObjectAt(index);
  }


  DfObjectType getObjectType() const {
    return (DfObjectType)objType;
  }



  // -- iterator --
};



class DfDataFrame {
  std::unordered_map<std::string, DfColumn> columns;

public:
  ~DfDataFrame() {

  }

  DfDataFrame(DfDataFrame& src) {
    
  }

  DfDataFrame(const std::initializer_list<std::pair<std::string, DfColumn>>& _columns) {
    
  }

  DfDataFrame(const std::initializer_list<std::string>& columnNames) {
    
  }



  DfColumn& operator[](const char* name) {

  }

  DfRow& loc(int index) {
    
  }
  

  


  DfProcess select(const char* sql) const;

  DfProcess where(const char* sql) const;

  DfProcess groupBy(const char* sql) const;

  DfProcess orderBy(const char* sql) const;

  DfProcess with(DfDataFrame& other, const char* asName) const;
};




class DfRow {
  std::unordered_map<std::string, DfColumn*>* columns;
  int index;
};



class DfProcess {
  std::unordered_map<std::string, DfDataFrame*> dataFrames;
  std::unordered_map<std::string, DfColumn*> columns;

  int targetedIndex;

  DfProcess(std::list<DfDataFrame>& _dataFrames, std::list<DfColumn>& _columns) {
    
  }

public:

  DfProcess& select(const char* sql);

  DfProcess& where(const char* sql);

  DfProcess& groupBy(const char* sql);

  DfProcess& orderBy(const char* sql);

  DfProcess& with(DfDataFrame& other, const char* asName);



  operator DfColumn() {

  }

  operator DfRow() {
    
  }

  operator DfDataFrame() {
    
  }
};





DfDataFrame DfReadCsv(const char* csvFile) {

}

DfDataFrame DfReadExcel(const char* excelFile) {

}

DfDataFrame DfReadXml(const char* xmlFile) {

}

#ifdef _MYHTML2_H_

DfDataFrame DfReadHtml() {

}

#endif


#endif // _MY_DATA_FRAME_HPP_