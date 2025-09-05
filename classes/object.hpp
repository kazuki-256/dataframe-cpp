/* df_object_t: a data targetor to edit data

features:
1. owns data or target to column data
2. constant data type, not allowed to change type after create
3. edit targeted data
4. convert data
5. std::cout support

*/

#pragma once


#include "../config.hpp"
#include "value.hpp"

#include <vector>





class df_null_t {
public:
  df_null_t() {};
} DF_NULL;




class df_object_t {
  friend class df_mem_block_t;
  friend class df_column_t;
  
  bool*     target_null = NULL;
  void*     target_date = NULL;
  df_type_t target_type = DF_TYPE_INT32;

  df_value_t  preloaded   = 0;
  bool        self_memory = false;
  bool        lock_state  = false;

  std::vector<std::string>* category_titles = NULL;





  inline void set_target(void* target, df_value_load_callback_t loader) {
    target = target;
    preloaded = loader(target);
  }

  inline void set_target(void* target, df_type_t target_type) {
    set_target(target, df_value_get_load_callback(target_type));
    data_type = target_type;
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
    new (target) std::string(const_string);
    preloaded = df_value_load_struct(target);
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
    if (data_type == DF_TYPE_TEXT) {
      *(std::string*)target = const_string;
      return *this;
    }

    std::string str = const_string;
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

  // == is_locked ==

  inline bool is_locked() const {
    return lock_state;
  }

  inline bool is_targeter() const {
    return owns_memory == false;
  }

  inline bool is_null() const {
    return *target_null;
  }


  // == get type ==

  inline df_type_t get_type() const {
    return data_type;
  }

  // == string ==

  inline std::string to_string() const {
    if (is_null()) {
      return "null";
    }
    return (std::string)*this;
  }

  friend inline std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << object.to_string();
  }
};






