/* df_object_t: a data targetor to edit data

features:
1. owns data or target to column data
2. constant data type, not allowed to change type after create
3. edit targeted data
4. convert data
5. std::cout support

*/

#include <iostream>
#ifndef _DF_TYPE_OBJECT_HPP_
#define _DF_TYPE_OBJECT_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPE_BYTE_HPP_
#include "value.hpp"
#endif




class df_null_t {
public:
  df_null_t() {};
} DF_NULL;





class df_object_t {
  friend class df_mem_block_t;
  friend class df_column_t;
  
  void* memory;         // target's memory, could be owns or column memory
  bool owns_memory;     // is the memory owned by this object?
  df_value_t value;     // preloaded memory value
  df_type_t data_type;  // data type

  std::vector<std::string>* category_titles;

  df_object_t() {
    category_titles = NULL;
  }

  inline void set_target(void* target, df_type_t target_type) {
    memory = target;
    df_value_load(value, memory, target_type);

    owns_memory = false;
    data_type = target_type;
  }


  inline void init(df_type_t as_type) {
    data_type = as_type;
    memory = malloc(DF_MAX_TYPE_SIZE);
    owns_memory = true;
  }

public:
  // == destroy ==

  ~df_object_t() {
    df_debug2("delete");

    if (owns_memory) {
      df_value_release(value, data_type);
      free(memory);
    }
  }


  // == init ==

  template<typename T>
  df_object_t(const T& const_value) {
    df_debug2("create object %s", df_type_get_string(df_type_get_type<T>));

    init(df_type_get_type<T>);
    new (memory) T(const_value);
    df_value_load(value, memory, data_type);
  }

  df_object_t(const char* const_string) {
    df_debug2("create object TEXT");

    init(DF_TYPE_TEXT);
    new (memory) df_string_t(const_string);
    df_value_load(value, memory, DF_TYPE_TEXT);
  }

  df_object_t(const char*&) = delete;
  df_object_t(const df_object_t&) = delete;



  // == setter ==

  template<typename SRC>
  df_object_t& operator=(SRC src) {
    df_value_t src_value;
    df_value_load(src_value, &src, df_type_get_type<SRC>);
    
    df_value_write(src_value, df_type_get_type<SRC>, memory, data_type);
    df_value_load(value, memory, data_type);
    return *this;
  }

  df_object_t& operator=(const char* const_string) {
    df_string_t str = const_string;
    df_value_t src_value;
    src_value.as_string = &str;

    df_value_write(src_value, DF_TYPE_TEXT, memory, data_type);
    df_value_load(value, memory, data_type);
    return *this;
  }

  df_object_t& operator=(const df_object_t& other) {
    df_value_write(other.value, other.data_type, memory, data_type);
    df_value_load(value, memory, data_type);
    return *this;
  }

  df_object_t& operator=(const char*&) = delete;
  df_object_t& operator=(uint8_t) = delete;
  df_object_t& operator=(short) = delete;



  // == convert data ==

  template<typename T>
  operator T() const {
    df_debug2("get object from type %s to %s", df_type_get_string(data_type), df_type_get_string(df_type_get_type<T>));

    T output;
    df_value_write(value, data_type, &output, df_type_get_type<T>);
    return output;
  }

  operator std::string() const {
    df_debug2("format %s -> TEXT", df_type_get_string(data_type));

    df_string_t output;
    df_value_write(value, data_type, &output, DF_TYPE_TEXT);
    return output.value_or("null");
  }


  // == get type ==

  df_type_t get_type() const {
    return data_type;
  }

  // == std::cout ==

  friend std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << (std::string)object;
  }
};








#endif // _DF_TYPE_OBJECT_HPP_
