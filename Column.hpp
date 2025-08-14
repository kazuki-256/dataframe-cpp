#ifndef _DF_COLUMN_HPP_
#define _DF_COLUMN_HPP_


#ifndef _DF_OBJECT_HPP_
#include "Object.hpp"
#endif


#include <list>


typedef DfObject& (*DfRowGetByString(void* userptr, const char* colName);
typedef DfObject& (*DfRowGetByIndex(void* userptr, int index);

typedwd int (*DfRowGetLength(void* userptr);

typedef void (*DfRowRelease)(void* userptr);



class DfRow {
  void* userptr;

  DfRowGetByString funcGetByString;
  DfRowGetByIndex funcGetByIndex;

  DfRowGetLength funcGetLength;

  DfRowRelease funcRelease;


  DfRow(void* _userptr,
        DfRowGetByString getByString,
        DfRowGetByIndex getByIndex,
        DfRowGetLength getLength,
        DfRowRelease release)
  {
    userptr = userptr;
    funcGetByString = getByString;
    funcGetByIndex = getByIndex;
    funcGetLength = getLength;
    funcRelease = release;
  }

public:
  ~DfRow() {
    if (userptr) release(userptr)
  }

  DfObject& operator[](const char* colName) {
    return getByString(userptr, colName);
  }

  DfObject& operator[](int index) {
    return getByIndex(userptr, index);
  }

  int getLength() const {
    return getLength();
  }
};



class DfObjectBlock {
  friend class DfColumn;

  DfObject* objects;
  int lowRange;
  int highRange;



  // create block without any settings
  DfObjectBlock(int _lowRange, int _highRange) {
    objects = new DfObject[_highRange - _lowRange + 1];
    lowRange = _lowRange;
    highRange = _highRange;
  }

  // copy for other array
  DfObjectBlock(const DfObject* _objects, int _length, int _lowRange, int _highRange) {
    objects = new DfObject[_length];
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
    objects = new DfObject[_objects.size()];
    lowRange = _lowRange;
    highRange = _highRange;

    int index = 0;
    for (const DfObject& obj : _objects) {
      objects[index++] = obj;
    }
  }



  void fillTypes(DfObjectType objType, int start, int length) {
    DfObject* out = objects + start - lowRange;
    DfObject* end = out + length;

    while (out < end) {
      *(out++) = DfObject(objType);
    }
  }


  // return a editable object, use real position
  DfObject& operator[](int index) {
    return *(objects + index - lowRange);
  }

public:
  ~DfObjectBlock() {
    if (objects) {
      delete[] objects;
    }
  }

  DfObjectBlock(DfObjectBlock&& other) {
    objects = other.objects;
    other.objects = NULL;

    lowRange = other.lowRange;
    highRange = other.highRange;
  }
};



class DfColumn {
  friend int main(int argc, char** argv);

  std::list<DfObjectBlock>* blocks;
  std::list<DfColumn*> beForegined; // SQL foregined by who (not usable)
  DfColumn* foreginTo;              // SQL foregin key (not usable)

  int length;
  uint8_t objType;
  bool isHidden;        // not usable currectly
  bool isReadonly;      // not usable
  bool isOwnsBlocks;    // not usable

  
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



  

  DfColumn mul(double number) const {
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_BOOLEAN) {
      throw DfException("not a number column!");
    }


    DfColumn column(length, DF_OBJTYPE_NUMBER);

    DfObjectBlock& last = column.blocks->back();
    int index = 0;

    for (DfObject& object : *this) {
      last.objects[index++] = (double)object * number;
    }
    return column;
  }




  // create column with uninited objects
  DfColumn(int _length, DfObjectType _objType) {
    init(isReadonly);

    length = 0;
    objType = _objType;

    blocks->push_back(DfObjectBlock(0, _length - 1));
    length = _length;
  }

public:

  ~DfColumn() {
    if (isOwnsBlocks) {
      delete blocks;
    }
  }

  DfColumn(const DfColumn& src) {
    init(isReadonly);

    length = 0;   // will set by merge()
    objType = src.objType;

    mergeWith(src);
  }

  DfColumn(const std::initializer_list<DfObject>& _objects, bool _isReadonly = false) {
    init(isReadonly);

    length = _objects.size();
    if (length == 0) {
      objType = DF_OBJTYPE_UNDEFINED;
      return;
    }
    
    objType = _objects.begin()->getObjectType();

    blocks->push_back(DfObjectBlock(_objects, 0, length - 1));
  }

  DfColumn(DfObjectType _objType = DF_OBJTYPE_UNDEFINED, bool _isReadOnly = false) {
    init(isReadonly);

    length = 0;
    objType = _objType;
  }


  
  // == get information ==

  inline DfObjectType getObjectType() const {
    return (DfObjectType)objType;
  }

  inline int getLength() const {
    return length;
  }



  // == iterator ==

  class Iterator {
    std::list<DfObjectBlock>::iterator blockIterator;
    int index;

    inline Iterator& next() {
      index++;
      if (index > blockIterator->highRange) {
        blockIterator++;
      }
      return *this;
    }

  public:
    Iterator(std::list<DfObjectBlock>* blocks) {
      blockIterator = blocks->begin();
      index = 0;
    }
    Iterator(int length) {
      index = length;
    }

    inline DfObject& operator*() {
      return (*blockIterator)[index];
    }

    inline bool operator!=(const Iterator& other) {
      return index < other.index;
    }

    inline Iterator& operator++() {
      return next();
    }

    inline Iterator& operator++(int) {
      return next();
    }
  };


  inline Iterator begin() const {
    return Iterator(blocks);
  }

  inline Iterator end() const {
    return Iterator(length);
  }



  // == operator ==

  // = move =

  DfColumn& operator=(DfColumn& other) {
    if (blocks) {
      delete blocks;
    }

    blocks = other.blocks;
    other.blocks = NULL;

    length = other.length;
    objType = other.objType;

    return *this;
  }


  // = addObject =

  DfColumn& addObject(const DfObject& object) {
    printf("addObject1: \n");
    if (objType != object.getObjectType()) {
      if (objType != DF_OBJTYPE_UNDEFINED) {
        throw DfException("couldn't add %d type object to %d type column!", object.getObjectType(), objType);
      }
      objType = object.getObjectType();
    }

    DfObjectBlock& last = *blocks->end();
    if (last.highRange >= length) {
      last[length++] = object;
      return *this;
    }

    blocks->push_back(DfObjectBlock(&object, 1, length, length + 7));
    length++;
    return *this;
  }

  DfColumn& addObject(DfObject&& object) {
    if (objType != object.getObjectType()) {
      if (objType != DF_OBJTYPE_UNDEFINED) {
        throw DfException("couldn't add %d type object to %d type column!", object.getObjectType(), objType);
      }
      objType = object.getObjectType();
    }

    DfObjectBlock& last = blocks->back();
    if (last.highRange >= length) {
      last[length++] = object;
      return *this;
    }

    blocks->push_back(DfObjectBlock(length, length + 7));
    blocks->back().objects[0] = object;
    blocks->back().fillTypes(getObjectType(), length + 1, 7);
    length++;
    return *this;
  }


  // = getObject =

  inline const DfObject& operator[](int index) const {
    return getObjectAt(index);
  }

  inline DfObject& operator[](int index) {
    if (index >= length) {
      addObject(DfObject(getObjectType()));
      return blocks->back()[length-1];
    }
    return getObjectAt(index);
  }



  // = merge =

  DfColumn& mergeWith(const DfColumn& other) {
    blocks->push_back(DfObjectBlock(length, length + other.length - 1));
    length += other.length;

    DfObjectBlock& last = blocks->back();
    int index = 0;

    for (DfObject& object : other) {
      last.objects[index++] = object;
    }
    return *this;
  }

  DfColumn getMerged(const DfColumn& other) const {
    DfColumn newColumn(length + other.length, getObjectType());

    DfObjectBlock& last = newColumn.blocks->back();
    int index = 0;

    for (DfObject& object : *this) {
      last.objects[index++] = object;
    }
    for (DfObject& object : other) {
      last.objects[index++] = object;
    }
    return newColumn;
  }



  // == values plus ==

  DfColumn operator+(const DfColumn& other) const {
    DfColumn newColumn(getObjectType());

    newColumn.blocks->push_back(DfObjectBlock(0, length + other.length - 1));
    newColumn.length = length + other.length;

    DfObjectBlock& last = newColumn.blocks->back();
    int index = 0;

    for (DfObject& object : *this) {
      last.objects[index++] = object;
    }
    for (DfObject& object : other) {
      last.objects[index++] = object;
    }
    return newColumn;
  }


  // = values mul =

  DfColumn operator*(double number) const {
    return mul(number);
  }
  
  DfColumn operator*(int number) const {
    return mul((double)number);
  }

  DfColumn operator*(bool boolean) const {
    return mul(boolean ? 1.0 : 0.0);
  }

  DfColumn operator*(DfColumn& other) const {
    if (objType != DF_OBJTYPE_NUMBER && objType != DF_OBJTYPE_BOOLEAN) {
      throw DfException("not a number column!");
    }
    if (other.objType != DF_OBJTYPE_NUMBER && other.objType != DF_OBJTYPE_BOOLEAN) {
      throw DfException("not a number column!");
    }


    DfColumn column(length, DF_OBJTYPE_NUMBER);

    DfObjectBlock& last = column.blocks->back();
    int index = 0;

    auto iterA = begin();
    auto iterB = other.begin();

    for (auto END = end(); iterA != END; iterA++, iterB++) {
      last.objects[index++] = (double)(*iterA) * (double)(*iterB);
    }
    return column;
  }



  // = print =

  void print(FILE* fp = stdout) const {
    fputs("column\n", fp);
    for (DfObject& object : *this) {
      fputs(object.toString().c_str(), fp);
      fputc('\n', fp);
    }
  }
};




#endif // _DF_COLUMN_HPP_