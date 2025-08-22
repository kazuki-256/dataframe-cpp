#ifndef _DF_TYPE_OBJECT_HPP_
#define _DF_TYPE_OBJECT_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPE_BYTE_HPP_
#include "byte.hpp"
#endif




class df_null_t {
public:
  df_null_t() {};
} DF_NULL;





class df_object_t {
  friend class df_mem_block_t;
  friend class df_column_t;
  
  void* mem;
  df_type_t type;
  bool owns_mem;
  std::vector<std::string>* category_titles;

  df_object_t() {
    category_titles = NULL;
  }

  inline void set_target(void* target, df_type_t target_type) {
    mem = target;
    type = target_type;
    owns_mem = false;
  }


  inline void init(df_type_t as_type) {
    type = as_type;
    mem = malloc(DF_MAX_TYPE_SIZE);
    owns_mem = true;
  }

  inline int handle_type(df_type_t dest_type) {
    // same type -> pass
    // number type -> pass (df_mem_set<> can handle the convert)
    if (type == dest_type && df_type_is_number(type)) {
      return 0;
    }

    // owns mem -> desktroy old type -> pass
    if (owns_mem) {
      df_debug2("change outside-object from type %s to %s!\n", df_type_get_string(type), df_type_get_string(dest_type));

      df_mem_reuse(mem, type, dest_type);
      type = dest_type;
      return 0;
    }
    return -1;
  }

public:
  ~df_object_t() {
    df_debug2("delete");

    if (owns_mem) {
      df_mem_release(mem, type);
      free(mem);
    }
  }

  template<typename T>
  df_object_t(const T& value) {
    df_debug2("create object %s", df_type_get_string(df_type_get_type<T>));

    init(df_type_get_type<T>);
    new (mem) T(value);
  }

  df_object_t(const char* value) {
    df_debug2("create object %s", df_type_get_string(df_type_get_type<T>));

    init(DF_TYPE_TEXT);
    new (mem) df_string_t(value);
  }

  df_object_t(const char*&) = delete;


  template<typename T>
  df_object_t& operator=(T value) {
    if (handle_type(df_type_get_type<T>)) {
      df_debug6("couldn't set object as different types when targeting dataframe or column!");
      return *this;
    }

    df_mem_convert(&value, df_type_get_type<T>, mem, type);
    return *this;
  }

  df_object_t& operator=(const char* value) {
    if (handle_type(DF_TYPE_TEXT)) {
      df_debug6("couldn't set object as different types when targeting dataframe or column!");
      return *this;
    }

    df_string_t str = value;
    df_mem_convert(&str, DF_TYPE_TEXT, mem, type);
    return *this;
  }

  df_object_t& operator=(const char*&) = delete;
  df_object_t& operator=(uint8_t) = delete;
  df_object_t& operator=(short) = delete;



  template<typename T>
  operator T() const {
    df_debug2("get object from type %s to %s", df_type_get_string(type), df_type_get_string(df_type_get_type<T>));

    T output;
    df_mem_convert(mem, type, &output, df_type_get_type<T>);
    return output;
  }

  operator std::string() const {
    df_debug2("format %s -> TEXT", df_type_get_string(type));

    df_string_t output;
    df_mem_convert(mem, type, &output, DF_TYPE_TEXT);
    return output.value_or("null");
  }

  df_type_t get_type() const {
    return type;
  }

  friend std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << (std::string)object;
  }
};








#endif // _DF_TYPE_OBJECT_HPP_
