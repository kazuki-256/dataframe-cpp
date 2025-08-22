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

  int usage;
  int capacity;
  df_type_t type;

  ///////
  // create block without any settings
  df_mem_block_t(df_type_t type, int capacity) {
    this->type = type;
    this->capacity = capacity;
  }


  static df_mem_block_t* create(df_type_t type, int size) {
    void* mem = malloc(sizeof(df_mem_block_t) + size * df_type_get_size(type));
    return new (mem) df_mem_block_t(type, size);
  }




  // copy for other array
  void fill(int start, const void* source_objects, int length);

  void fill(int start, const std::initializer_list<df_object_t>& source_objects);

  inline void* get_start() const {
    return (void*)this + sizeof(df_mem_block_t);
  }

public:
  ~df_mem_block_t() {
    if (!df_type_is_struct(type)) {
      return;
    }

    if (type == DF_TEXT) {
      for (df_string_t* p = (df_string_t*)get_start(), *END = p + usage; p < END; p++) {
        p->reset();
      }
      return;
    }
    df_debug6("deleting unknown structed type");
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
  TLinkableList<df_mem_block_t> chunks;
  int length;
  uint8_t type;

  std::set<df_column_t*> be_foregined;  // SQL foregined by who (not usable)
  df_column_t* foregin_to = NULL;       // SQL foregin key (not usable)

  
  df_object_t& get_object_at(int index) const;

  df_mem_block_t* extend_chunk(int size, int* lessing_size);


  template<typename T>
  void typed_merge(const df_column_t& other);



public:

  ~df_column_t() noexcept(false);


  df_column_t(const std::initializer_list<df_object_t>& objects);

    // advaned constructor
  df_column_t(df_type_t objtype, int capacity = 0);


  
  // === get information ===

  inline df_type_t get_type() const;

  inline int get_chunk_count() const;

  inline int get_length() const;



  // === iterator ===

  class iterator;

  class const_iterator;


  inline iterator begin();

  inline iterator end();

  inline const_iterator begin() const;

  inline const_iterator end() const;



  // === operator ===

  // == move ==

  df_column_t(df_column_t&& src);

  df_column_t& operator=(df_column_t&& src);

  // == copy ==

  df_column_t(const df_column_t& src);

  df_column_t operator=(const df_column_t& src);



  // == set_foregin ==

  df_column_t& set_foregin(df_column_t* column);


  // == add_object ==

  df_column_t& add_object(const df_object_t& object);

  df_column_t& add_object(df_object_t&& object);


  // == get_object ==

  inline const df_object_t& operator[](int index) const;

  inline df_object_t& operator[](int index);



  // == merge ==

  template<typename other_t>
  df_column_t& merge_with(const df_column_t& other);

  template<typename other_t>
  df_column_t merge_to(const df_column_t& other) const;


  // == print ==

  std::ostream& write_stream(std::ostream& os, const char* colname) const;

  friend std::ostream& operator<<(std::ostream& os, const df_column_t& column);



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




#endif // _DF_COLUMN_HPP_
