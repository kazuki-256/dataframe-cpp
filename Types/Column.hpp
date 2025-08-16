#ifndef _DF_COLUMN_HPP_
#define _DF_COLUMN_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_OBJECT_HPP_
#include "object.hpp"
#endif

#ifndef _LINKABLE_HPP_
#include "../linkable/linkable.hpp"
#endif

#include <list>
#include <set>



class df_process;
template<typename object_t> class df_column;



// ==== df_object_chunk ====

template<typename object_t>
class df_object_chunk : TLinkable {
  template<typename T> friend class df_column;

  df_object<object_t>* objects;
  int usage;
  int capacity;


  inline void init(int _capacity) {
    capacity = _capacity * DF_CHUNK_GROWTH_FACTOR;
    objects = (df_object<object_t>*)calloc(capacity, sizeof(df_object<object_t>));
    usage = 0;
  }


  // create block without any settings
  df_object_chunk(int capacity) {
    init(capacity);
  }



  // copy for other array
  void fill(int start, const df_object<object_t>* source_objects, int length) {
    df_debug2("fill1 from %p to %p (length: %d, current: %d, capacity: %d)",
        source_objects, objects, length, usage, capacity);

    // -- fastest array copies --
    df_object<>* out = objects + start;
    const df_object<>* in  = source_objects;
    const df_object<>* end = source_objects + length;

    while (out < end) {
      out->from_copy(*in);
      out++, in++;
    }
    usage = MAX(usage, start + length);
  }

  void fill(int start, const std::initializer_list<df_object<object_t>>& source_objects) {
    df_debug2("fill1 from %p to %p (length: %d, current: %d, capacity: %d)",
        source_objects, objects, source_objects.size(), usage, capacity);

    for (const df_object<object_t>& object : source_objects) {
      df_debug2("debug: %s", object.c_str());
      objects[start++].from_copy(object);
    }
    usage = MAX(usage, start);
    df_debug2("usage: %d, capacity: %d", usage, capacity);
  }

  inline void release_objects_only() {
    for (df_object<object_t>* ptr = objects, *END = objects + usage; ptr < END; ptr++) {
      ptr->release();
    }
  }


  inline void from_move(df_object_chunk& other) {
    objects = other.objects;
    other.objects = NULL;

    usage = other.usage;
    capacity = other.capacity;
  }

public:
  ~df_object_chunk() {
    if (objects) {
      release_objects_only();
      free(objects);
    }
  }


  df_object_chunk() {
    objects = NULL;
  }


  // == move ==

  df_object_chunk(df_object_chunk&& other) {
    df_debug2("df_object_chunk::move1");
    from_move(other);
  }

  df_object_chunk& operator=(df_object_chunk&& other) {
    df_debug2("df_object_chunk::move2");

    if (objects) {
      release_objects_only();
      free(objects);
    }
    
    from_move(other);
    return *this;
  }


  // == copy ==

  df_object_chunk(const df_object_chunk& other) {
    df_debug2("df_object_chunk::copy1");

    init(other.usage);

    fill(0, other.objects, other.usage);
    usage = other.usage;
  }

  df_object_chunk operator=(const df_object_chunk& other) noexcept {
    df_debug2("df_object_chunk::copy2");

    if (objects) {
      release_objects_only();

      if (other.usage < capacity) {
        goto label_fill;
      }

      free(objects);
    }
    
    capacity = other.usage * DF_CHUNK_GROWTH_FACTOR;
    objects = (df_object<object_t>*)calloc(capacity, sizeof(df_object<object_t>));

  label_fill:

    fill(0, other.objects, other.usage);
    usage = other.usage;
    return *this;
  }
};






// ==== df_column ==

template<typename object_t = DF_DEFAULT_TYPE>
class df_column {
  template<typename T> friend class df_column;

  TLinkableList<df_object_chunk<object_t>> chunks;
  int length;
  uint8_t type;

  std::set<df_column<object_t>*> be_foregined;  // SQL foregined by who (not usable)
  df_column<object_t>* foregin_to = NULL;       // SQL foregin key (not usable)

  
  df_object<object_t>& get_object_at(int index) const {
    // -- negivate find --
    if (index < 0) {
      index = -index;

      if (index <= 0 && index > length) {
        throw df_exception_out_of_index();
      }

      for (TLinkable* iter = chunks.tlBack(); iter != NULL; iter = iter->tlPrev()) {
        df_object_chunk<object_t>* chunk = (df_object_chunk<object_t>*)iter;

        if (index <= chunk->usage) {
          return chunk->objects[chunk->usage - index];
        }
        index -= chunk->usage;
      }
      throw df_exception("chunk didn't seted");
    }
    
    // -- positive find --
    if (index >= length) {
      throw df_exception_out_of_index();
    }

    for (const df_object_chunk<object_t>* chunk : chunks) {
      if (index < chunk->usage) {
        return chunk->objects[index];
      }
      index -= chunk->usage;
    }
    throw df_exception("chunk didn't seted");
  }

  df_object_chunk<object_t>* extend_chunk(int size, int* lessing_size = DF_DEFAULT_RETURN_POINTER) {
    // == way 1: use currently chunk ==
    if (!chunks.tlFront()) {
      chunks.tlAdd(new df_object_chunk<object_t>((length + size) * DF_CHUNK_GROWTH_FACTOR));
      *lessing_size = size;
      return chunks.tlBack();
    }

    // == way 2: use currently + new chunk ==
    df_object_chunk<object_t>& last = *chunks.tlBack();

    int request = last.usage + size;
    if (request >= last.capacity) {
      chunks.tlAdd(new df_object_chunk<object_t>(length + request - last.capacity));
    }

    *lessing_size = MIN(size, last.capacity - last.usage);
    return &last;
  }


  template<typename T>
  void typed_merge(const df_column<T>& other) {
    df_column<T>& casted = *this;

    // sure memory
    int lessing_size;
    df_object_chunk<T>* chunk = casted.extend_chunk(other.length, &lessing_size);

    // write chunk[-2] or chunk[-1]
    df_debug3("typed_merge usage=%d lessing=%d src=%d", chunk->usage, lessing_size, other.length);

    auto iter = other.begin();
    for (const int END = chunk->usage + lessing_size; chunk->usage < END; ) {
      chunk->objects[chunk->usage++] = *iter;
      iter++;
    }

    // write chunk[-1]
    if (other.length > lessing_size) {
      chunk = casted.chunks.tlBack();
      
      for (; iter != other.end(); iter++) {
        chunk->objects[chunk->usage++] = *iter;
      }
    }
    casted.length += other.length;
  }



public:

  ~df_column() noexcept(false) {
    df_debug3("delete df_column %p", this);

    if (!be_foregined.empty()) {
      throw df_exception("couldn't delete column because column has been foregined!");
    }
    if (foregin_to) {
      foregin_to->be_foregined.erase(this);
    }

    if (type == DF_TYPE_STRING) {
      TLinkableList<df_object_chunk<df_string>>* casted = (TLinkableList<df_object_chunk<df_string>>*)&chunks;
      casted->tlClear();
    }
  }



  df_column(const std::initializer_list<df_object<object_t>>& objects) {
    df_debug3("create df_column1: %p", this);

    type = df_get_type_v<object_t>;
    length = objects.size();

    if (length == 0) {
      return;
    }

    chunks.tlAdd(new df_object_chunk<object_t>(length));
    df_object_chunk<object_t>& chunk = *chunks.tlBack();

    chunk.fill(0, objects);
  }


  df_column(int capacity = 0) {
    df_debug3("create df_column2");

    type = df_get_type_v<object_t>;
    length = 0;

    if (capacity) {
      chunks.tlAdd(new df_object_chunk<object_t>(capacity));
    }
  }

    // advaned constructor
  df_column(df_type objtype, int capacity = 0) {
    df_debug3("create df_column3");

    type = objtype;
    length = 0;

    if (capacity) {
      chunks.tlAdd(new df_object_chunk<object_t>(capacity));
    }
  }


  
  // === get information ===

  inline df_type get_type() const {
    return (df_type)type;
  }

  inline int get_chunk_count() const {
    return chunks.size();
  }

  inline int get_length() const {
    return length;
  }



  // === iterator ===

  class iterator {
  protected:

    inline iterator& next() {
      index++;
      if (index >= chunk->usage) {
        df_debug3("%d", index);
        chunk = (df_object_chunk<object_t>*)chunk->tlNext();
        index = 0;
      }
      return *this;
    }

    df_object<object_t>& get() const {
      return chunk->objects[index];
    }

  public:
    df_object_chunk<object_t>* chunk;
    int index;

    iterator(df_object_chunk<object_t>* _chunk) {
      chunk = _chunk;
      index = 0;
    }



    inline df_object<object_t>& operator*() const {
      return get();
    }

    inline bool operator!=(const iterator& other) const {
      return chunk != NULL;
    }

    inline iterator& operator++() {
      return next();
    }

    inline iterator& operator++(int) {
      return next();
    }
  };


  class const_iterator : public iterator {
  public:
    const_iterator(df_object_chunk<object_t>* _chunk) : iterator(_chunk) {}

    const_iterator() : iterator(NULL) {}

  };


  inline iterator begin() {
    df_debug3("begin()");
    return iterator(chunks.tlFront());
  }

  inline iterator end() {
    df_debug3("end()");
    return iterator(NULL);
  }

  inline const_iterator begin() const {
    df_debug3("const begin()");
    return const_iterator(chunks.tlFront());
  }

  inline const_iterator end() const {
    df_debug3("const end()");
    return const_iterator(NULL);
  }






  // === operator ===

  // == move ==

  df_column(df_column&& src) {
    df_debug3("df_column move1 %p -> %p\n", src, this);


    chunks = std::move(src.chunks);

    length = src.length;
    type = src.type;
  }

  df_column& operator=(df_column&& src) {
    df_debug3("df_column move2");

    chunks = std::move(src.chunks);

    length = src.length;
    type = src.type;

    return *this;
  }

  // == copy ==

  df_column(const df_column& src) {
    df_debug3("df_column copy1: %p -> %p", &src, this);
    length = 0;   // will set by merge()
    type = src.type;

    merge_with(src);
  }

  df_column operator=(const df_column& src) {
    df_debug3("df_column copy2: %p -> %p", &src, this);

    if (!be_foregined.empty()) {
      throw df_exception("couldn't delete column because column has been foregined!");
    }
    if (foregin_to) {
      foregin_to->be_foregined.erase(this);
    }
    chunks.clear();

    length = 0;   // will set by merge()
    type = src.type;

    merge_with(src);
  }


  // == cast ==

  template<typename Dest>
  operator df_column<Dest>&() {
    return *(df_column<Dest>*)this;
  }

  template<typename Dest>
  operator const df_column<Dest>&() const {
    return *(const df_column<Dest>*)this;
  }



  // == set_foregin ==

  df_column& set_foregin(df_column* column) {
    foregin_to = column;
    column->be_foregined.insert(column);
    return *this;
  }


  // == add_object ==

  df_column& add_object(const df_object<object_t>& object) {
    df_object_chunk<object_t>& chunk = extend_chunk(1);
    chunk.objects[chunk.usage++] = object;
    length++;
    return *this;
  }

  df_column& add_object(df_object<object_t>&& object) {
    df_object_chunk<object_t>& chunk = extend_chunk(1);
    chunk.objects[chunk.usage++] = std::move(object);
    length++;
    return *this;
  }


  // == get_object ==

  inline const df_object<object_t>& operator[](int index) const {
    return get_object_at(index);
  }

  inline df_object<object_t>& operator[](int index) {
    if (index >= length) {
      df_object_chunk<object_t>* chunk = extend_chunk(1);
      
      return chunk->objects[chunk->usage++];
    }
    return get_object_at(index);
  }



  // == merge ==

  template<typename other_t>
  df_column<object_t>& merge_with(const df_column<other_t>& other) {
    if (type != other.type) {
      throw df_exception("couldn't merge column since they are different types!");
    }

    switch (type) {
      case DF_TYPE_STRING:
        df_debug3("merge df_column<df_string>");
        typed_merge<df_string>(other);
        break;
      default:
        df_debug3("merge df_column<df_undefined> %d", type);
        typed_merge<df_undefined>(other);
        break;
    }
    return *this;
  }

  template<typename other_t>
  df_column<object_t> merge_to(const df_column<other_t>& other) const {
    if (type != other.type) {
      throw df_exception("couldn't merge column since they are different types!");
    }

    df_column<object_t> new_column(get_type(), length + other.length);


    switch (type) {
      case DF_TYPE_STRING:
        typed_merge<df_string>(other);
        break;
      default:
        typed_merge<df_undefined>(other);
        break;
    }
    return new_column;
  }



  // == print ==

  std::ostream& write_stream(std::ostream& os, const char* colname) const {
    os << "| " << colname << " |\n|";

    for (const df_object<object_t>& object : *this) {
      os << object.c_str(type) << " |\n";
    }
    return os;
  }

  friend std::ostream& operator<<(std::ostream& os, const df_column<object_t>& col) {
    col.write_stream(os, "x");
    return os;
  }




  // == SQL ==

  df_process as(const char* name) const;


  // == vector ==

  df_process operator+(int num) const;
  df_process operator+(df_number num) const;

  df_process operator-(int num) const;
  df_process operator-(df_number num) const;

  df_process operator*(int num) const;
  df_process operator*(df_number num) const;
  
  df_process operator/(int num) const;
  df_process operator/(df_number num) const;
  
  df_process operator%(int num) const;
  df_process operator%(df_number num) const;

  // also in Math.hpp
  // df_process& DfSqrt(df_process)
  // df_process& DfAvg(df_process)
  // ...
};




#endif // _DF_COLUMN_HPP_