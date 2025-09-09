#pragma once

#include "../config.hpp"
#include "object.hpp"

#include <list>
#include <set>


#ifndef _LINKABLE_HPP_
#include "../linkable/linkable.hpp"
#endif




// ==== df_mem_block_t ====

class df_mem_block_t : TLinkable {
private:
  // create block without any settings
  df_mem_block_t(df_type_t data_type, long capacity);


protected:
  friend class df_mem_iterator_t;
  friend class df_object_iterator_t;
  friend class df_const_object_iterator_t;

  friend class df_column_t;
  friend class df_column_time_t;

  friend df_column_t df_range_int32(int, int, int);
  friend df_column_t df_range_int64(long, long, long);
  friend df_column_t df_range_datetime(df_date_t, df_date_t, df_interval_t);



  df_type_t data_type;
  long usage;
  long capacity;


  static df_mem_block_t* create(df_type_t data_type, long capacity);


  uint8_t* init_values(long start, long length, int size_per_data);


  // copy for other array
  inline void fill_unsafe(long start, df_const_mem_iterator_t& iter,
      df_type_t src_type, long length, int size_per_data,
      df_value_load_callback_t loader, df_value_write_callback_t writer);

  inline void fill_unsafe(long start, const std::initializer_list<df_object_t>& source_objects);



  uint8_t* get_null_start() const;

  uint8_t* get_null_at(long index) const;

  uint8_t* get_data_start() const;

  uint8_t* get_data_at(long index) const;


public:
  ~df_mem_block_t();

  // == move ==
  df_mem_block_t(df_mem_block_t&& other) = delete;
  df_mem_block_t& operator=(df_mem_block_t&& other) = delete;

  // == copy ==
  df_mem_block_t(const df_mem_block_t& other) = delete;
  df_mem_block_t operator=(const df_mem_block_t& other) = delete;
};




// ==== df_mem_iterator_t ====

class df_mem_iterator_t {
  protected:
    df_mem_block_t* block = NULL;
    uint8_t*  null_p    = NULL;
    uint8_t*  data_p    = NULL;
    uint8_t*  data_end  = NULL;

    int size_per_data = 0;



    void basic_copy(const df_mem_iterator_t& other);

    void forward(long offset);

  public:
    df_mem_iterator_t(df_mem_block_t* init_block);

    constexpr df_mem_iterator_t();

    // == copy ==

    df_mem_iterator_t(const df_mem_iterator_t& other);

    df_mem_iterator_t& operator=(const df_mem_iterator_t& other);


    bool* get_null() const;

    uint8_t* get_data() const;


    df_mem_iterator_t& operator++();

    df_mem_iterator_t& operator++(int);

    df_mem_iterator_t& operator+=(long offset);


    bool is_end() const;

    bool operator!=(const df_mem_iterator_t& other) const;
  };



// ==== df_const_mem_iterator_t ====

class df_const_mem_iterator_t : public df_mem_iterator_t {
public:
  df_const_mem_iterator_t(df_mem_block_t* block);

  constexpr df_const_mem_iterator_t();

  df_const_mem_iterator_t& operator+=(long offset);
};



// ==== df_object_iterator_t ====

class df_object_iterator_t : public df_mem_iterator_t {
protected:
  df_value_load_callback_t loader = NULL;
  df_object_t user_object;

public:
  df_object_iterator_t(df_mem_block_t* block);

  constexpr df_object_iterator_t();


  df_object_t& operator*();

  df_object_iterator_t& operator+=(long offset);
};



// ==== df_const_object_iterator_t ====

class df_const_object_iterator_t : public df_object_iterator_t {
public:
  df_const_object_iterator_t(df_mem_block_t* block);

  constexpr df_const_object_iterator_t();

  df_const_mem_iterator_t& operator=(const df_const_mem_iterator_t& other);


  const df_object_t& operator*();

  df_const_object_iterator_t& operator+=(long offset);
};








// ==== df_column_t ====

class df_column_t {
  friend df_column_t df_range_int32(int, int, int);
  friend df_column_t df_range_int64(long, long, long);
  friend df_column_t df_range_datetime(df_date_t, df_date_t, df_interval_t);
  friend df_column_t df_range_date(df_date_t, df_date_t, df_interval_t);
  friend df_column_t df_range_time(df_date_t, df_date_t, df_interval_t);

protected:
  TLinkableList<df_mem_block_t> blocks;
  long length = 0;
  df_type_t data_type = DF_TYPE_UINT8;

  // std::set<df_column_t*> be_foregined;  // SQL foregined by who (not usable)
  // df_column_t* foregin_to = NULL;       // SQL foregin key (not usable)

  
  df_object_t get_object_at(long index) const;



  df_mem_block_t* expand_front(long sure_size, long& lessing);

  df_mem_block_t* expand_back(long sure_size, long& lessing);



  df_mem_block_t* init_normal(df_type_t data_type, long length, long start_capacity);

  template<typename DEST, typename T>
  void init_by_typed(df_type_t data_type, const std::initializer_list<T>& const_list);

  df_column_t(int); // real-do-nothing init



  void copy(const df_column_t& src);

  void move(df_column_t& src) noexcept;

public:

  ~df_column_t() noexcept(false);
  


  constexpr df_column_t();

  df_column_t(df_type_t data_type, long start_capacity = DF_DEFAULT_COLUMN_SMALL_START_CAPACITY);

  df_column_t(const std::initializer_list<df_object_t>& objects);


  // == copy ==

  df_column_t(const df_column_t& src);

  df_column_t operator=(const df_column_t& src);


  // == move ==

  df_column_t(df_column_t&& src) noexcept;

  df_column_t& operator=(df_column_t&& src) noexcept;


  
  // === get information ===

  df_type_t get_data_type() const;

  int get_block_count() const;

  long get_length() const;



  // == set_foregin ==

  df_column_t& set_foregin(df_column_t* column);


  // == push_front ==

  df_column_t& push_front(const df_object_t& object);

  // == push_back ==

  df_column_t& push_back(const df_object_t& object);


  // == get_object ==

  const df_object_t operator[](long index) const {
    return get_object_at(index);
  }

  df_object_t operator[](long index) {
    return get_object_at(index);
  }



  // == merge ==

  df_column_t& merge_with(const df_column_t& other);

  df_column_t merge_to(const df_column_t& other) const;



  // == convert ==

  template<typename T>
  operator std::vector<T>();



  // === iterator ===

  df_mem_iterator_t mem_begin();

  df_mem_iterator_t mem_end();

  df_const_mem_iterator_t mem_begin() const;

  df_const_mem_iterator_t mem_end() const;


  df_object_iterator_t begin();

  df_object_iterator_t end();

  df_const_object_iterator_t begin() const;

  df_const_object_iterator_t end() const;


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
};



std::ostream& operator<<(std::ostream& os, const std::pair<std::string, df_column_t>& named_column);







class df_column_int32_t : public df_column_t {
public:
  df_column_int32_t(const std::initializer_list<int>& int32_list);
};

class df_column_int64_t : public df_column_t {
public:
  df_column_int64_t(const std::initializer_list<long>& int64_list);
};



class df_column_float32_t : public df_column_t {
public:
  df_column_float32_t(const std::initializer_list<float>& float32_list);
};

class df_column_float64_t : public df_column_t {
public:
  df_column_float64_t(const std::initializer_list<double>& float64_list);
};



class df_column_text_t : public df_column_t {
public:
  df_column_text_t(const std::initializer_list<const char*>& text_list);

  df_column_text_t(const std::initializer_list<std::string>& text_list);
};



class df_column_date_t : public df_column_t {
public:
  df_column_date_t(const std::initializer_list<df_date_t>& date_list);
};

class df_column_time_t : public df_column_t {
public:
  df_column_time_t(const std::initializer_list<const char*>& text_list);
};

class df_column_datetime_t : public df_column_t {
public:
  df_column_datetime_t(const std::initializer_list<df_date_t>& datetime_list);
};


