#ifndef _DF_OBJECT_HPP_
#define _DF_OBJECT_HPP_


#ifndef _DF_EXCEPTION_HPP_
#include "Exception.hpp"
#endif
#ifndef _DF_DATE_HPP_
#include "Date.hpp"
#endif


#include <string>




std::string dfRepr(const std::string& s) {
  std::string out = "\"";
  for (char c : s) {
    switch (c) {
      case '\n': out += "\\n"; break;
      case '\t': out += "\\t"; break;
      case '\r': out += "\\r"; break;
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      default:
        if (isprint(c)) {
          out += c;
        }

        char buf[5];
        snprintf(buf, 5, "\\x%02x", c);
        out += buf;
    }
  }
  out += "\"";
  return out;
}





typedef enum DfType {
  DF_OBJTYPE_UNDEFINED,

  DF_OBJTYPE_POINTER,
  DF_OBJTYPE_BOOLEAN,
  DF_OBJTYPE_STRING,
  DF_OBJTYPE_NUMBER,
  DF_OBJTYPE_DATE,

} DfType;




class DfObjectChunk;


class DfObject {
  friend class DfObjectChunk;

  union U{
    void* asPointer;
    bool asBoolean;
    char* asString;
    double asNumber;
    DfDate asDate;

    U() {};
    ~U() {};
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

  void release() {
    if (objType == DF_OBJTYPE_STRING && data.asString) {
      free(data.asString);
      data.asString = NULL;
    }
  }

public:
  ~DfObject() {
    release();
  }

  // create empty object
  DfObject(DfType _objType = DF_OBJTYPE_UNDEFINED) {
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
  DfObject(const char* str) {
    init(DF_OBJTYPE_STRING, false);

    extra = (int)strlen(str);
    if (extra == 0) {
      data.asString = NULL;
      return;
    }

    data.asString = (char*)malloc(extra + 1);
    strcpy(data.asString, str);
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
  DfObject(DfDate date) {
    init(DF_OBJTYPE_DATE, false);
    data.asDate = date;
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
    if (objType == DF_OBJTYPE_NUMBER) {
      return data.asNumber ? true : false;
    }
    if (objType == DF_OBJTYPE_BOOLEAN || objType == DF_OBJTYPE_UNDEFINED) {
      return data.asBoolean;
    }
    throw DfException("couldn't get data by different type!");
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
    if (objType == DF_OBJTYPE_BOOLEAN) {
      return data.asBoolean ? 1.0 : 0.0;
    }
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asNumber;
  }

  // get number
  operator long() const {
    if (objType == DF_OBJTYPE_BOOLEAN) {
      return data.asBoolean ? 1.0 : 0.0;
    }
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return (long)data.asNumber;
  }

  // get number
  operator int() const {
    if (objType == DF_OBJTYPE_BOOLEAN) {
      return data.asBoolean ? 1 : 0;
    }
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return (int)data.asNumber;
  }

  // get date
  operator DfDate() const {
    if (objType != DF_OBJTYPE_DATE && objType != DF_OBJTYPE_UNDEFINED) {
      throw DfException("couldn't get data by different type!");
    }
    return data.asDate;
  }


  
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



  DfType getType() const {
    return (DfType)objType;
  }

  bool isNull() const {
    return null;
  }

  std::string toString() {
    if (objType == DF_OBJTYPE_UNDEFINED || isNull()) {
      return "null";
    }

    if (objType == DF_OBJTYPE_BOOLEAN) {
      return data.asBoolean ? "true" : "false";
    }
    if (objType == DF_OBJTYPE_STRING) {
      return data.asString ? data.asString : NULL;
    }

    std::string str(32, 0);
    if (objType == DF_OBJTYPE_DATE) {
      return data.asDate.toString(&str[0], "%Y-%m-%d %H:%M:%S");
    }
    if (objType == DF_OBJTYPE_POINTER) {
      snprintf(&str[0], str.size(), "%p", data.asPointer);
      return str;
    }
    if (objType == DF_OBJTYPE_NUMBER) {
      snprintf(&str[0], str.size(), "%g", data.asNumber);
      return str;
    }
    return "null";
  }
};




#endif // _DF_OBJECT_HPP_