#ifndef _DF_TYPE_OBJECT_HPP_
#define _DF_TYPE_OBJECT_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPE_BYTE_HPP_
#include "byte.hpp"
#endif







class df_object_t {
  void* data;
  df_type_t type;
  bool owns_data;
  std::vector<std::string>* category_titles;

  df_object_t(void* target, df_type_t target_type) {
    data = target;
    type = target_type;
    owns_data = false;
  }

  inline void init(df_type_t as_type) {
    type = as_type;
    data = malloc(DF_MAX_TYPE_SIZE);
    owns_data = true;
  }

public:
  ~df_object_t() {
    df_debug1("delete");

    if (owns_data == false) {
      return;
    }

    if (type == DF_TEXT) {
      ((std::string*)data)->~basic_string();
    }
    free(data);
  }

  template<typename T>
  df_object_t(const T& value) {
    df_debug1("create object (%s)", df_type_get_string(df_type_get_type<T>));

    init(df_type_get_type<T>);
    *(T*)data = value;
  }


  template<typename T>
  df_object_t& operator=(T& value) {
    if (!owns_data && type != df_type_get_type<T>) {
      df_debug6("couldn't set object as different types when targeting dataframe or column!");
      return *this;
    }

    df_mem_set(data, type, value);
    return *this;
  }

  template<typename T>
  operator T() const {
    T value;
    df_mem_get(data, type, value);
    return value;
  }

  friend std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    stream << (std::string)object;
    return stream;
  }
};




<<<<<<< HEAD:Types/Object.hpp




#endif // _DF_TYPE_OBJECT_HPP_
=======
#endif // _DF_OBJECT_HPP_
>>>>>>> e91bc2cb611defaf90ba1af8ca7646f80da722a7:Types/object.hpp
