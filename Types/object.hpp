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
  
  void* target;         // target's target, could be owns or column target
  df_value_t preloaded; // preloaded target preloaded
  bool owns_memory;     // is the target owned by this object?
  df_type_t data_type;  // data type

  std::vector<std::string>* category_titles;

  inline df_object_t() {
    category_titles = NULL;
  }

  inline void set_target(void* target, df_type_t target_type, df_value_load_callback_t loader) {
    target = target;
    preloaded = loader(target);

    owns_memory = false;
    data_type = target_type;
  }

  inline void set_target(void* target, df_type_t target_type) {
    set_target(target, target_type, df_value_get_load_callback(target_type));
  }


  inline void init(df_type_t as_type) {
    data_type = as_type;
    target = malloc(DF_MAX_TYPE_SIZE);
    owns_memory = true;
  }

public:
  // == destroy ==

  inline ~df_object_t() {
    df_debug2("delete");

    if (owns_memory) {
      df_value_release(preloaded, data_type);
      free(target);
    }
  }


  // == init ==

  template<typename T>
  inline df_object_t(const T& const_value) {
    df_debug2("create object %s", df_type_get_string(df_type_get_type<T>));

    init(df_type_get_type<T>);
    new (target) T(const_value);
    preloaded = df_value_load(target, data_type);
  }

  inline df_object_t(const char* const_string) {
    df_debug2("create object TEXT");

    init(DF_TYPE_TEXT);
    new (target) df_string_t(const_string);
    preloaded = df_value_load_string(target);
  }

  df_object_t(const char*&) = delete;
  df_object_t(const df_object_t&) = delete;



  // == move / copy ==

  df_object_t(df_object_t&& other) {
    target = other.target;
    preloaded = other.preloaded;
    owns_memory = other.owns_memory;
    data_type = other.data_type;

    other.owns_memory = false;
  }


  inline df_object_t& operator=(const df_object_t& other) {
    preloaded = df_value_write(other.preloaded, other.data_type, target, data_type);
    return *this;
  }


  // == setter ==

  template<typename SRC>
  inline df_object_t& operator=(SRC src) {
    df_value_t src_value = df_value_load(&src, df_type_get_type<SRC>);

    preloaded = df_value_write(src_value, df_type_get_type<SRC>, target, data_type);
    return *this;
  }

  inline df_object_t& operator=(const char* const_string) {
    df_string_t str = const_string;
    df_value_t src_value = &str;

    preloaded = df_value_write(src_value, DF_TYPE_TEXT, target, data_type);
    return *this;
  }

  df_object_t& operator=(const char*&) = delete;
  df_object_t& operator=(uint8_t) = delete;
  df_object_t& operator=(short) = delete;



  // == convert data ==

  template<typename T>
  inline operator T() const {
    df_debug2("convert object from type %s to %s", df_type_get_string(data_type), df_type_get_string(df_type_get_type<T>));

    T output;
    df_value_write(preloaded, data_type, &output, df_type_get_type<T>);
    return output;
  }

  inline operator std::string() const {
    df_debug2("format %s -> TEXT", df_type_get_string(data_type));

    df_string_t output;
    df_value_write(preloaded, data_type, &output, DF_TYPE_TEXT);
    return output.value_or("null");
  }


  // == is valid / invalid ==

  inline bool is_valid() const {
    return data_type != DF_TYPE_NULL;
  }

  inline bool is_invalid() const {
    return data_type == DF_TYPE_NULL;
  }


  // == get type ==

  inline df_type_t get_type() const {
    return data_type;
  }

  // == std::cout ==

  friend inline std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << (std::string)object;
  }
};








#endif // _DF_TYPE_OBJECT_HPP_
