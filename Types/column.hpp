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



// ==== df_mem_block_t ====

class df_mem_block_t : TLinkable {
  friend class df_column_t;
  friend class df_column_text_t;
  friend class df_column_time_t;

  df_type_t data_type;
  long usage;
  long capacity;

  // create block without any settings
  df_mem_block_t(df_type_t data_type, long capacity) {
    this->data_type = data_type;

    this->capacity = capacity;
    this->usage = 0;
  }


  static df_mem_block_t* create(df_type_t data_type, long capacity) {
    const int size_per_data = df_type_get_size(data_type);
    const long final_capacity = (capacity + 1024) & ~1023;

    void* memory = malloc(sizeof(df_mem_block_t) + final_capacity * size_per_data);
    return new (memory) df_mem_block_t(data_type, final_capacity);
  }




  // copy for other array
  void fill_unsafe(long start, const uint8_t* src, df_type_t src_type, long length) {
    const int DATA_SIZE = df_type_get_size(data_type);
    const int DATA_TYPE_ID = df_type_get_typeid(data_type);
    const int SRC_TYPE_ID = df_type_get_typeid(src_type);

    // == get callbacks ==

    df_value_load_callback_t loader = DF_VALUE_LOAD_CALLBACKS[SRC_TYPE_ID];

    df_value_write_callback_t writer =
      DF_VALUE_WRITE_CALLBACKS[SRC_TYPE_ID][DATA_TYPE_ID];
    
    if (loader == NULL || writer == NULL) {
      df_debug6("couldn't convert %s to %s", df_typeid_get_string(DATA_TYPE_ID), df_typeid_get_string(SRC_TYPE_ID));
      return;
    }

    // == copies ==

    uint8_t* dest = get_start() + start * DATA_SIZE;
    uint8_t* end = dest + length * DATA_SIZE;

    for (; dest < end; dest += DATA_SIZE, src += DATA_SIZE) {
      df_value_t value = loader((void*)src);
      writer(value, dest);
    }
    usage = DF_MAX(usage, start + length);
  }

  void fill_unsafe(long start, const std::initializer_list<df_object_t>& source_objects) {
    const int DATA_SIZE = df_type_get_size(data_type);
    const int DATA_TYPE_ID = df_type_get_typeid(data_type);

    // == get callbacks ==

    df_value_load_callback_t default_loader =
      DF_VALUE_LOAD_CALLBACKS[DATA_TYPE_ID];

    df_value_write_callback_t default_writer =
      DF_VALUE_WRITE_CALLBACKS[DATA_TYPE_ID][DATA_TYPE_ID];


    if (default_loader == NULL || default_writer == NULL) {
      const char* TYPE_NAME = df_typeid_get_string(DATA_TYPE_ID);
      df_debug3("invalid convertion %s -> %s", TYPE_NAME, TYPE_NAME);
      return;
    }

    // == write values ==

    uint8_t* dest = get_start() + start * DATA_SIZE;
    for (const df_object_t& object : source_objects) {
      if (object.data_type != data_type) {
        df_debug3("detacted initing %s object in %s column, auto converting...", df_type_get_string(object.data_type), df_typeid_get_string(DATA_TYPE_ID));

        df_value_write_callback_t writer =
          DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(object.data_type)][DATA_TYPE_ID];
        
        if (default_writer == NULL) {
          df_debug3("couldn't convert %s to %s", df_type_get_string(object.data_type), df_typeid_get_string(DATA_TYPE_ID));
          continue;
        }

        writer(object.preloaded, dest);
        dest += DATA_SIZE;
        continue;
      }

      default_writer(object.preloaded, dest);
      dest += DATA_SIZE;
    }

    // set variable
    usage = DF_MAX(usage, start + source_objects.size());
  }

  inline uint8_t* get_start() const {
    return (uint8_t*)this + sizeof(df_mem_block_t);
  }

  inline uint8_t* get_end() const {
    return (uint8_t*)this + sizeof(df_mem_block_t) + usage * df_type_get_size(data_type);
  }

public:
  ~df_mem_block_t() {
    if (!df_type_is_struct(data_type)) {
      return;
    }

    if (data_type == DF_TYPE_TEXT) {
      constexpr int SIZE = df_type_get_size(DF_TYPE_TEXT);
      for (uint8_t* p = get_start(), *END = p + usage; p < END; p += SIZE) {
        df_value_release_string_mem(p);
      }
      return;
    }
    df_debug6("deleting unknown structed data_type");
  }

  // == move ==
  df_mem_block_t(df_mem_block_t&& other) = delete;
  df_mem_block_t& operator=(df_mem_block_t&& other) = delete;

  // == copy ==
  df_mem_block_t(const df_mem_block_t& other) = delete;
  df_mem_block_t operator=(const df_mem_block_t& other) = delete;
};






// ==== df_column_t ==

class df_column_t {
protected:
  TLinkableList<df_mem_block_t> blocks;
  long length;
  df_type_t data_type;

  std::set<df_column_t*> be_foregined;  // SQL foregined by who (not usable)
  df_column_t* foregin_to = NULL;       // SQL foregin key (not usable)


  static uint8_t* outside_write_data(df_column_t& column, long length) {
    df_mem_block_t* block = column.blocks.tlBack();
    column.length = block->usage = length;

    return block->get_start();
  }

  
  df_object_t get_object_at(long index) const {
    const int DATA_SIZE = df_type_get_size(data_type);
    df_mem_block_t* block;
    df_object_t object;

    // == rfind ==

    if (index < 0) {
      index = -index;
      for (block = blocks.tlBack(); block != NULL; block = (df_mem_block_t*)block->tlPrev()) {
        if (index <= block->usage) {
          object.set_target(block->get_start() + (block->usage - index) * DATA_SIZE, data_type);
          return object;
        }
        index -= block->usage;
      }

      // ERROR: not found
      throw df_exception_out_of_index();
    }

    // == find ==
    
    for (block = blocks.tlFront(); block != NULL; block = (df_mem_block_t*)block->tlNext()) {
      if (index < block->usage) {
        object.set_target(block->get_start() + index * DATA_SIZE, data_type);
        return object;
      }
      index -= block->usage;
    }

    // ERROR: not found
    throw df_exception_out_of_index();
  }



  df_mem_block_t* expand_front(long sure_size, long& lessing) {
    df_mem_block_t* block = blocks.tlFront();

    if (block == NULL) {
      block = blocks.tlInsertBefore(0, df_mem_block_t::create(data_type, sure_size));
      lessing = block->capacity;
      return block;
    }

    lessing = block->capacity - block->usage;
    if (sure_size > lessing) {
      blocks.tlInsertBefore(0, df_mem_block_t::create(data_type, sure_size - lessing));
    }
    else if (lessing > sure_size) {
      lessing = sure_size;
    }

    return block;
  }

  df_mem_block_t* expand_back(long sure_size, long& lessing) {
    df_mem_block_t* block = blocks.tlBack();

    if (block == NULL) {
      block = blocks.tlAdd(df_mem_block_t::create(data_type, sure_size));
      lessing = sure_size;
      return block;
    }

    lessing = block->capacity - block->usage;
    if (sure_size > lessing) {
      blocks.tlAdd(df_mem_block_t::create(data_type, sure_size - lessing));
    }
    else if (lessing > sure_size) {
      lessing = sure_size;
    }

    return block;
  }


  inline df_mem_block_t* init1(df_type_t data_type, long length, long start_capacity) {
    df_debug3("create %s column with length %ld, start capacity %ld", df_type_get_string(data_type), length, start_capacity);

    this->data_type = data_type;
    this->length = length;

    return blocks.tlAdd(df_mem_block_t::create(data_type, start_capacity));
  }

  template<typename DEST, typename T>
  inline void init2(df_type_t data_type, const std::initializer_list<T>& const_list) {
    df_mem_block_t* block = init1(data_type, const_list.size(), const_list.size() * 2);
    block->usage = length;

    uint8_t* p = block->get_start();
    const int DATA_SIZE = df_type_get_size(data_type);

    for (T const_value : const_list) {
      new (p) DEST(const_value);
      p += DATA_SIZE;
    }
  }

  df_column_t() {}

public:

  ~df_column_t() noexcept(false) {
    df_debug3("delete column with %d blocks %d objects", blocks.tlLength(), length);
  }


  df_column_t(df_type_t data_type, long start_capacity = DF_DEFAULT_COLUMN_SMALL_START_CAPACITY) {
    init1(data_type, 0, start_capacity);
  }


  df_column_t(const std::initializer_list<df_object_t>& objects) {
    df_debug3("create column with %lu objects", objects.size());
    
    df_mem_block_t* block = init1(objects.begin()->get_type(), objects.size(), objects.size() * 2);
    block->fill_unsafe(0, objects);
  }


  
  // === get information ===

  inline df_type_t get_data_type() const {
    return data_type;
  }

  inline int get_block_count() const {
    return blocks.tlLength();
  }

  inline long get_length() const {
    return length;
  }



  // === iterator ===

  class mem_iterator {
  protected:
    df_mem_block_t* block;
    uint8_t* p;
    uint8_t* end;
    int size_per_data;

  public:
    mem_iterator(df_mem_block_t* block) {
      this->block = block;
      this->size_per_data = df_type_get_size(block->data_type);

      this->p = block->get_start();
      this->end = p + block->usage * size_per_data;
    }

    mem_iterator(int) {
      this->block = NULL;
    }


    inline uint8_t* operator*() const {
      return p;
    }

    inline mem_iterator& operator++() {
      p += size_per_data;
      if (p >= end) {
        block = (df_mem_block_t*)block->tlNext();
        
        if (block) {
          p = block->get_start();
          end = p + block->usage * size_per_data;
        }
      }
      return *this;
    }

    inline mem_iterator operator++(int) {
      mem_iterator old = *this;
      ++(*this);
      return old;
    }

    inline bool operator!=(const mem_iterator& other) const {
      return block != NULL;
    }
  };

  class const_mem_iterator : public mem_iterator {
  public:
    const_mem_iterator(df_mem_block_t* block) : mem_iterator(block) {}

    const_mem_iterator(int) : mem_iterator(0) {}

    inline const uint8_t* operator*() const {
      return p;
    }

  };

  class iterator : public mem_iterator {
    df_object_t user_object;
  public:
    iterator(df_mem_block_t* block) : mem_iterator(block) {}

    iterator(int) : mem_iterator(0) {}


    inline df_object_t& operator*() {
      user_object.set_target(p, block->data_type);
      return user_object;
    }
  };

  class const_iterator : public mem_iterator {
    df_object_t user_object;
  public:
    const_iterator(df_mem_block_t* block) : mem_iterator(block) {}

    const_iterator(int) : mem_iterator(0) {}


    inline const df_object_t& operator*() {
      user_object.set_target(p, block->data_type);
      return user_object;
    }
  };



  inline mem_iterator mem_begin() {
    return mem_iterator(blocks.tlFront());
  }

  inline mem_iterator mem_end() {
    return mem_iterator(0);
  }

  inline const_mem_iterator mem_begin() const {
    return const_mem_iterator(blocks.tlFront());
  }

  inline const_mem_iterator mem_end() const {
    return const_mem_iterator(0);
  }


  inline iterator begin() {
    return iterator(blocks.tlFront());
  }

  inline iterator end() {
    return iterator(0);
  }

  inline const_iterator begin() const {
    return const_iterator(blocks.tlFront());
  }

  inline const_iterator end() const {
    return const_iterator(0);
  }



  // === operator ===

  // == move ==

  df_column_t(df_column_t&& src) {
    df_debug3("move column with %d blocks", src.blocks.tlLength());

    this->blocks = std::move(src.blocks);
    this->length = src.length;
    this->data_type = src.data_type;

    this->be_foregined = std::move(src.be_foregined);
    this->foregin_to = src.foregin_to;

    src.length = 0;
    src.data_type = DF_TYPE_NULL;
  }

  df_column_t& operator=(df_column_t&& src) {
    df_debug3("move column with %d blocks", src.blocks.tlLength());

    if (this == &src) {
      return *this;
    }

    this->blocks = std::move(src.blocks);
    this->length = src.length;
    this->data_type = src.data_type;

    this->be_foregined = std::move(src.be_foregined);
    this->foregin_to = src.foregin_to;

    src.length = 0;
    src.data_type = DF_TYPE_NULL;
    return *this;
  }

  // == copy ==

  df_column_t(const df_column_t& src) {
    df_debug3("copy column with %d blocks", src.blocks.tlLength());

    this->data_type = src.data_type;
    
    df_mem_block_t* new_block = df_mem_block_t::create(src.data_type, src.length);
    blocks.tlAdd(new_block);

    for (df_mem_block_t* block : src.blocks) {
      new_block->fill_unsafe(length, block->get_start(), src.data_type, block->usage);
      length += block->usage;
    }

    this->be_foregined = src.be_foregined;
    this->foregin_to = src.foregin_to;
  }

  df_column_t operator=(const df_column_t& src) {
    df_debug3("copy column with %d blocks", src.blocks.tlLength());

    if (this == &src) {
      return *this;
    }

    blocks.tlClear();

    data_type = src.data_type;
    length = 0;
    
    df_mem_block_t* new_block = df_mem_block_t::create(src.data_type, src.length);
    blocks.tlAdd(new_block);

    for (df_mem_block_t* block : src.blocks) {
      new_block->fill_unsafe(length, block->get_start(), src.data_type, block->usage);
      length += block->usage;
    }

    this->be_foregined = src.be_foregined;
    this->foregin_to = src.foregin_to;
    return *this;
  }



  // == set_foregin ==

  df_column_t& set_foregin(df_column_t* column);


  // == push_front ==

  df_column_t& push_front(const df_object_t& object) {
    long lessing;
    expand_front(1, lessing);

    df_mem_block_t* block = blocks.tlFront();
    uint8_t* start = block->get_start();
    int DATA_SIZE = df_type_get_size(data_type);

    // == move exists data ==

    for (uint8_t* p1 = block->get_end(), *p2 = p1; p2 != start; ) {
      p1 -= DATA_SIZE;
      memcpy(p1, p2, DATA_SIZE);
      p2 -= DATA_SIZE;
    }

    // == write ==

    df_value_write(object.preloaded, object.data_type, start, data_type);
    block->usage++;
    this->length++;

    return *this;
  }

  // == push_back ==

  inline df_column_t& push_back(const df_object_t& object) {
    long lessing;
    expand_back(1, lessing);
    df_mem_block_t* block = blocks.tlBack();

    df_value_write(object.preloaded, object.data_type, block->get_end(), data_type);
    block->usage++;
    this->length++;

    return *this;
  }


  // == get_object ==

  inline const df_object_t operator[](long index) const {
    return get_object_at(index);
  }

  inline df_object_t operator[](long index) {
    return get_object_at(index);
  }



  // == merge ==

  df_column_t& merge_with(const df_column_t& other) {
    const int DATA_SIZE = df_type_get_size(data_type);
    long lessing;
    df_mem_block_t* block = expand_back(other.length, lessing);

    // == get callbacks ==

    df_value_load_callback_t loader = DF_VALUE_LOAD_CALLBACKS[df_type_get_typeid(other.data_type)];
    df_value_write_callback_t writer = DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(other.data_type)][df_type_get_typeid(this->data_type)];

    // null check

    // == copy (round 1: existing block) ==

    uint8_t* p = block->get_end();
    uint8_t* end = p + lessing * DATA_SIZE;
    const_mem_iterator iter = other.mem_begin();
    df_value_t value;

    while (p < end) {
      value = loader(*(iter++));
      writer(value, p);
      p += DATA_SIZE;
    }
    block->usage += lessing;

    // == copy (round 2: new block) ==

    if (lessing < other.length) {
      block = blocks.tlBack();
      p = block->get_start();

      while (iter != other.mem_end()) {
        value = loader(*(iter++));
        writer(value, p);
        p += DATA_SIZE;
      }
      block->usage = other.length - lessing;
    }

    this->length += other.length;
    return *this;
  }

  inline df_column_t merge_to(const df_column_t& other) const {
    df_column_t output(this->data_type, (this->length + other.length) * 2);

    return output.merge_with(*this).merge_with(other);
  }


  // == print ==

  std::ostream& write_stream(std::ostream& os, const char* colname) const {
    int DATA_TYPE_ID = df_type_get_typeid(data_type);

    // == get callbacks ==

    df_value_load_callback_t loader =
      DF_VALUE_LOAD_CALLBACKS[DATA_TYPE_ID];
    
    df_value_write_callback_t writer =
      DF_VALUE_WRITE_CALLBACKS[DATA_TYPE_ID][DF_TYPEID_TEXT];
    
    if (loader == NULL || writer == NULL) {
      df_debug6("couldn't print column with data_type %s", df_typeid_get_string(DATA_TYPE_ID));
      return os;
    }

    // == print ==

    df_string_t buffer;
    df_value_t value;

    os << "|  " << colname << "  |\n";
    for (mem_iterator it = mem_begin(); it != mem_end(); it++) {
      value = loader(*it);
      writer(value, &buffer);

      os << "| " << buffer.value_or("null") << " |\n";
    }
    return os;
  }

  friend inline std::ostream& operator<<(std::ostream& os, const df_column_t& column) {
    return column.write_stream(os, DF_DEFAULT_COLUMN_NAME);
  }




  // == make range ==

  static df_column_t make_int32_range(int start, int end, int interval);

  static df_column_t make_int64_range(long start, long end, long interval);



  static df_column_t make_random_int32_range(int min, int max, long length);

  static df_column_t make_random_int64_range(double min, double max, long length);

  static df_column_t make_random_float32_range(float min, float max, long length);

  static df_column_t make_random_float64_range(double min, double max, long length);
  

  static df_column_t make_date_range(df_date_t start, df_date_t end, df_interval_t interval);
  
  static df_column_t make_datetime_range(df_date_t start, df_date_t end, df_interval_t interval);



  // == SQL ==

  df_query_t as(const char* name) const;


  // == vector ==

  df_query_t operator+(long num) const;
  df_query_t operator+(double num) const;

  df_query_t operator-(long num) const;
  df_query_t operator-(double num) const;

  df_query_t operator*(long num) const;
  df_query_t operator*(double num) const;
  
  df_query_t operator/(long num) const;
  df_query_t operator/(double num) const;
  
  df_query_t operator%(long num) const;
  df_query_t operator%(double num) const;

  // also in Math.hpp
  // df_query_t& DfSqrt(df_query_t)
  // df_query_t& DfAvg(df_query_t)
  // ...
};



inline std::ostream& operator<<(std::ostream& os, const std::pair<std::string, df_column_t>& named_column) {
    return named_column.second.write_stream(os, named_column.first.c_str());
}







class df_column_int32_t : public df_column_t {
public:
  inline df_column_int32_t(const std::initializer_list<int>& int32_list) {
    init2<int>(DF_TYPE_INT32, int32_list);
  }

  static df_column_t range(int start, int end, int interval) {
    if (interval == 0) {
      throw df_exception_t("interval couldn't be 0");
    }
    
    long length = (end - start) / interval;
    if (length < 0) {
      throw df_exception_t("endless numbers");
    }

    df_column_t column(DF_TYPE_INT32, length);
    uint8_t* p = outside_write_data(column, length);
     
    for (; start < end; start += interval) {
      df_value_write_long_int(start, p);
      p += DF_TYPESIZE_INT32;
    }
    return column;
  }
};

class df_column_int64_t : public df_column_t {
public:
  inline df_column_int64_t(const std::initializer_list<long>& int64_list) {
    init2<long>(DF_TYPE_INT64, int64_list);
  }

  static df_column_t range(long start, long end, long interval) {
    if (interval == 0) {
      throw df_exception_t("interval couldn't be 0");
    }

    long length = (end - start) / interval;
    if (length < 0) {
      throw df_exception_t("endless numbers");
    }

    df_column_t column(DF_TYPE_INT64, length);
    uint8_t* p = outside_write_data(column, length);
     
    for (; start < end; start += interval) {
      df_value_write_long_long(start, p);
      p += DF_TYPESIZE_INT64;
    }
    return column;
  }
};



class df_column_float32_t : public df_column_t {
public:
  inline df_column_float32_t(const std::initializer_list<float>& float32_list) {
    init2<float>(DF_TYPE_FLOAT32, float32_list);
  }
};

class df_column_float64_t : public df_column_t {
public:
  inline df_column_float64_t(const std::initializer_list<double>& float64_list) {
    init2<double>(DF_TYPE_FLOAT64, float64_list);
  }
};



class df_column_text_t : public df_column_t {
public:
  inline df_column_text_t(const std::initializer_list<const char*>& text_list) {
    init2<df_string_t>(DF_TYPE_TEXT, text_list);
  }

  inline df_column_text_t(const std::initializer_list<std::string>& text_list) {
    init2<df_string_t>(DF_TYPE_TEXT, text_list);
  }
};



class df_column_date_t : public df_column_t {
public:
  inline df_column_date_t(const std::initializer_list<df_date_t>& date_list) {
    init2<df_date_t>(DF_TYPE_DATE, date_list);
  }

  static df_column_t range(df_date_t start, df_date_t end, df_interval_t interval) {
    int direction = interval.get_direction();
    if (direction == 0) {
      throw df_exception_t("interval couldn't be 0");
    }

    if (((time_t)end - (time_t)start) * direction < 0) {
      throw df_exception_t("endless numbers");
    }

    df_column_t column(DF_TYPE_DATE);
    
    for (; start <= end; start += interval) {
      column.push_back(start);
    }
    
    return column;
  }
};

class df_column_time_t : public df_column_t {
public:
  inline df_column_time_t(const std::initializer_list<std::string>& text_list) {
    df_mem_block_t* block = init1(DF_TYPE_TIME, text_list.size(), text_list.size() * 2);
    block->usage = length;
    
    uint8_t* p = block->get_start();
    const int DATA_SIZE = df_type_get_size(data_type);

    for (const std::string& const_text : text_list) {
      new (p) df_date_t(const_text.c_str(), DF_TIME_FORMAT);
      p += DATA_SIZE;
    }
  }

  static df_column_int32_t range(df_date_t start, df_date_t end, df_interval_t interval);
};

class df_column_datetime_t : public df_column_t {
public:
  inline df_column_datetime_t(const std::initializer_list<df_date_t>& datetime_list) {
    init2<df_date_t>(DF_TYPE_DATETIME, datetime_list);
  }

  static df_column_t range(df_date_t start, df_date_t end, df_interval_t interval);
};






#endif // _DF_COLUMN_HPP_
