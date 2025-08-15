#ifndef _DF_COLUMN_HPP_
#define _DF_COLUMN_HPP_


#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif


#ifndef _DF_OBJECT_HPP_
#include "Object.hpp"
#endif

#include <list>
#include <set>



class DfProcess;


\

class DfObjectChunk {
  friend class DfColumn;

  DfObject* objects;
  int usage;
  int capacity;


  inline void init(int _capacity) {
    capacity = _capacity + 8;
    objects = (DfObject*)calloc(capacity, sizeof(DfObject));
    usage = 0;
  }


  // create block without any settings
  DfObjectChunk(int capacity) {
    init(capacity);
  }



  // copy for other array
  void fill(int start, const DfObject* srcObjs, int length) {
    // -- fastest array copies --
    DfObject* out = objects + start;
    const DfObject* in  = srcObjs;
    const DfObject* end = srcObjs + length;

    while (out < end) {
      *out = *in;
      out++, in++;
    }
    usage = MAX(usage, start + length);
  }

  void fill(int start, const std::initializer_list<DfObject>& srcObjs) {
    for (const DfObject& obj : srcObjs) {
      objects[start++] = obj;
    }
    usage = MAX(usage, start);
    DfDebug2("usage: %d, start: %d", usage, start);
  }

  inline void releaseObjects() {
    for (DfObject* ptr = objects, *END = objects + usage; ptr < END; ptr++) {
      ptr->release();
    }
  }

public:
  ~DfObjectChunk() {
    if (objects) {
      releaseObjects();
      free(objects);
    }
  }


  DfObjectChunk() {
    objects = NULL;
  }


  // == move ==

  DfObjectChunk(DfObjectChunk&& other) {
    DfDebug2("DfObjectChunk::move1");
    init(other.usage);

    DfDebug2("other info: mem %p  usage %d  capacity %d", other.objects, other.usage, other.capacity);

    memcpy(objects, other.objects, other.usage * sizeof(DfObject));
    usage = other.usage;
  }

  DfObjectChunk& operator=(DfObjectChunk&& other) {
    DfDebug2("DfObjectChunk::move2");

    if (objects) {
      releaseObjects();
      free(objects);
    }

    objects = other.objects;
    other.objects = NULL;

    usage = other.usage;
    capacity = other.capacity;
    return *this;
  }


  // == copy ==

  DfObjectChunk(const DfObjectChunk& other) {
    DfDebug2("DfObjectChunk::copy1");

    init(other.usage);

    fill(0, other.objects, other.usage);
    usage = other.usage;
  }

  DfObjectChunk operator=(const DfObjectChunk& other) noexcept {
    DfDebug2("DfObjectChunk::copy2");

    if (objects) {
      releaseObjects();

      if (other.usage >= capacity) {
        free(objects);
      }
    }
    else {
      capacity = other.usage + 8;
      objects = (DfObject*)calloc(capacity, sizeof(DfObject));
    }

    fill(0, other.objects, other.usage);
    usage = other.usage;
  }
};



class DfColumn {
  std::list<DfObjectChunk> chunks;
  std::set<DfColumn*> beForegined;  // SQL foregined by who (not usable)
  DfColumn* foreginTo = NULL;       // SQL foregin key (not usable)

  int length;
  uint8_t objType;
  
  DfObject& _getObjectAt(int index) const {
    // -- negivate find --
    if (index < 0) {
      index = -index;

      if (index <= 0 && index > length) {
        throw DfExceptionOutOfIndex();
      }

      for (auto iter = chunks.rbegin(); iter != chunks.rend(); iter++) {
        if (index <= iter->usage) {
          return iter->objects[iter->usage - index];
        }
        index -= iter->usage;
      }
      throw DfException("chunk didn't seted");
    }
    
    // -- positive find --
    if (index >= length) {
      throw DfExceptionOutOfIndex();
    }

    for (const DfObjectChunk& chunk : chunks) {
      if (index < chunk.usage) {
        return chunk.objects[index];
      }
      index -= chunk.usage;
    }
    throw DfException("chunk didn't seted");
  }

  DfObjectChunk& _getChunkBack(DfType inObjType) {
    // == chunk types ==
    if (objType != inObjType) {
      if (objType != DF_OBJTYPE_UNDEFINED) {
        throw DfException("couldn't add %d type object to %d type column!", inObjType, objType);
      }
      objType = inObjType;
    }

    // == way 1 ==
    if (!chunks.empty()) {
      DfObjectChunk* last = &chunks.back();
      if (last->usage < last->capacity) {
        return *last;
      }
    }

    // == way 2 ==
    chunks.push_back(DfObjectChunk(length));
    return chunks.back();
  }

  DfObjectChunk* extendChunk(int size, int* lessingSize) {
    if (chunks.empty()) {
      chunks.push_back(DfObjectChunk(size));
      *lessingSize = size;
      return &chunks.back();
    }

    DfObjectChunk& last = chunks.back();

    int request = last.usage + size;
    if (request >= last.capacity) {
      chunks.push_back((request) - last.capacity);
    }

    *lessingSize = MIN(size, last.capacity - last.usage);
    return &last;
  }



  template<typename T>
  class _Iterator {
    T chunkIter;
    int index;

    inline _Iterator& next() {
      index++;
      if (index >= chunkIter->usage) {
        DfDebug3("%d", index);
        chunkIter++;
        index = 0;
      }
      return *this;
    }

  public:
    _Iterator(T&& _chunkIter) {
      chunkIter = std::move(_chunkIter);
      index = 0;
    }

    inline DfObject& operator*() {
      DfDebug3("test");
      return chunkIter->objects[index];
    }

    inline bool operator!=(const _Iterator<T>& other) {
      DfDebug3("check");
      return chunkIter != other.chunkIter;
    }

    inline _Iterator& operator++() {
      return next();
    }

    inline _Iterator& operator++(int) {
      return next();
    }
  };

public:

  ~DfColumn() noexcept(false) {
    DfDebug3("delete DfColumn %p", this);
    if (!beForegined.empty()) {
      throw DfException("couldn't delete column because column has been foregined!");
    }
    if (foreginTo) {
      foreginTo->beForegined.erase(this);
    }
  }



  DfColumn(const std::initializer_list<DfObject>& _objects) {
    DfDebug3("create DfColumn1: %p", this);

    length = _objects.size();
    if (length == 0) {
      objType = DF_OBJTYPE_UNDEFINED;
      return;
    }
    
    objType = _objects.begin()->getType();

    chunks.push_back(DfObjectChunk(length));
    DfObjectChunk& chunk = chunks.back();

    chunk.fill(0, _objects);
    DfDebug3("create DfColumn end!");
  }

  DfColumn(DfType _objType = DF_OBJTYPE_UNDEFINED, int capacity = 0) {
    DfDebug3("create DfColumn2");
    objType = _objType;
    length = 0;
    if (capacity) chunks.push_back(DfObjectChunk(capacity));
  }


  
  // === get information ===

  inline DfType getObjectType() const {
    return (DfType)objType;
  }

  inline int getChunkCouunt() const {
    return chunks.size();
  }

  inline int getLength() const {
    return length;
  }



  // === iterator ===

  typedef _Iterator<std::list<DfObjectChunk>::iterator> Iterator;
  typedef _Iterator<std::list<DfObjectChunk>::const_iterator> ConstIterator;


  inline Iterator begin() {
    DfDebug3("begin()");
    return Iterator(chunks.begin());
  }

  inline Iterator end() {
    DfDebug3("end()");
    return Iterator(chunks.end());
  }

  inline ConstIterator begin() const {
    DfDebug3("const begin()");
    return ConstIterator(chunks.begin());
  }

  inline ConstIterator end() const {
    DfDebug3("const end()");
    return ConstIterator(chunks.end());
  }



  // === operator ===

  // == move ==

  DfColumn(DfColumn&& src) {
    DfDebug3("DfColumn move1 %p -> %p\n", src, this);
    chunks = std::move(src.chunks);

    length = src.length;
    objType = src.objType;
  }

  DfColumn& operator=(DfColumn&& src) {
    DfDebug3("DfColumn move2");
    DfDebug3("src.chunk.size() = %d\n", src.chunks.size());

    chunks = std::move(src.chunks);

    length = src.length;
    objType = src.objType;

    return *this;
  }

  // == copy ==

  DfColumn(const DfColumn& src) {
    DfDebug3("DfColumn copy1: %p -> %p", &src, this);
    length = 0;   // will set by merge()
    objType = src.objType;

    mergeWith(src);
  }

  DfColumn operator=(const DfColumn& src) {
    DfDebug3("DfColumn copy2: %p -> %p", &src, this);

    if (!beForegined.empty()) {
      throw DfException("couldn't delete column because column has been foregined!");
    }
    if (foreginTo) {
      foreginTo->beForegined.erase(this);
    }
    chunks.clear();

    length = 0;   // will set by merge()
    objType = src.objType;

    mergeWith(src);
  }


  // == setForegin ==

  DfColumn& setForegin(DfColumn* column) {
    foreginTo = column;
    column->beForegined.insert(column);
    return *this;
  }


  // == addObject ==

  DfColumn& addObject(const DfObject& object) {
    DfObjectChunk& chunk = _getChunkBack(object.getType());
    chunk.objects[chunk.usage++] = object;
    length++;
    return *this;
  }

  DfColumn& addObject(DfObject&& object) {
    DfObjectChunk& chunk = _getChunkBack(object.getType());
    chunk.objects[chunk.usage++] = std::move(object);
    length++;
    return *this;
  }


  // == getObject ==

  inline const DfObject& operator[](int index) const {
    return _getObjectAt(index);
  }

  inline DfObject& operator[](int index) {
    if (index >= length) {
      DfObjectChunk& chunk = _getChunkBack(getObjectType());
      
      return chunk.objects[chunk.usage++];
    }
    return _getObjectAt(index);
  }



  // == merge ==

  DfColumn& mergeWith(const DfColumn& other) {
    chunks.push_back(DfObjectChunk(length));
    length += other.length;

    // sure memory
    int lessingSize;
    DfObjectChunk* chunk = extendChunk(other.length, &lessingSize);

    // write chunk[-2] or chunk[-1]
    DfDebug3("debug %d %d %d", chunk->usage, lessingSize, other.length);

    ConstIterator iter = other.begin();
    for (const int END = chunk->usage + lessingSize; chunk->usage < END; ) {
      chunk->objects[chunk->usage++] = *iter;
      iter++;
    }

    // write chunk[-1]
    if (other.length > lessingSize) {
      chunk = &chunks.back();
      
      for (; iter != other.chunks.end(); iter++) {
        chunk->objects[chunk->usage++] = *iter;
      }
    }
    
    return *this;
  }

  DfColumn mergeTo(const DfColumn& other) const {
    DfColumn newColumn(getObjectType(), length + other.length);

    DfObjectChunk& last = newColumn.chunks.back();

    for (DfObject& object : *this) {
      last.objects[newColumn.length++] = object;
    }
    for (DfObject& object : other) {
      last.objects[newColumn.length++] = object;
    }
    return newColumn;
  }



  // == print ==

  void print(FILE* fp = stdout) const {
    fputs("column\n", fp);
    for (DfObject& object : *this) {
      fputs(object.toString().c_str(), fp);
      fputc('\n', fp);
    }
  }



  // == SQL ==

  DfProcess as(const char* name) const;


  // == vector ==

  DfProcess operator+(int num) const;
  DfProcess operator+(double num) const;

  DfProcess operator-(int num) const;
  DfProcess operator-(double num) const;

  DfProcess operator*(int num) const;
  DfProcess operator*(double num) const;
  
  DfProcess operator/(int num) const;
  DfProcess operator/(double num) const;
  
  DfProcess operator%(int num) const;
  DfProcess operator%(double num) const;

  // also in Math.hpp
  // DfProcess& DfSqrt(DfProcess)
  // DfProcess& DfAvg(DfProcess)
  // ...
};




#endif // _DF_COLUMN_HPP_