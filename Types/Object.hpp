#ifndef _DF_OBJECT_HPP_
#define _DF_OBJECT_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_EXCEPTION_HPP_
#include "Exception.hpp"
#endif

#ifndef _DF_DATE_HPP_
#include "date.hpp"
#endif



#include <string>
#include <math.h>




// ==== types of object ==

typedef void* df_undefined;

typedef std::string df_string;
typedef void* df_pointer;
typedef double df_number;
typedef bool df_boolean;




class df_category {
  template<typename T> friend class df_object;
  friend class df_column<df_category>;
  friend class df_column<df_string>;

  int category;

public:
  df_category() = delete;

  bool operator==(const df_category& other) const {
    return category == other.category;
  }
  bool operator!=(const df_category& other) const {
    return category == other.category;
  }
};



// ==== types of enum ====

typedef enum df_type {
  DF_TYPE_POINTER,
  DF_TYPE_BOOLEAN,
  DF_TYPE_STRING,
  DF_TYPE_NUMBER,
  DF_TYPE_DATE,
  DF_TYPE_CATEGORY,   // this type have to create from df_column

} df_type;



template<typename T>
struct df_get_type {
    static constexpr df_type value =
      std::is_pointer_v<T> ? DF_TYPE_POINTER
      : std::is_same_v<T, df_boolean> ? DF_TYPE_BOOLEAN
      : std::is_same_v<T, df_string> ? DF_TYPE_STRING
      : std::is_same_v<T, df_date> ? DF_TYPE_DATE
      : std::is_same_v<T, df_category> ? DF_TYPE_CATEGORY
      : DF_TYPE_NUMBER;
};

template<typename T>
inline constexpr df_type df_get_type_v = df_get_type<T>::value;






template<typename T = DF_DEFAULT_TYPE>
class df_object {
  static_assert(sizeof(T) <= 8 || std::is_same_v<df_string, T>, "T size could not over 8 bytes!");
  static_assert(!std::is_const_v<T>, "not allowed to use const value");

  friend class df_object_chunk<T>;
  friend int main(int argc, char** argv);

  static char STATIC_BUFFER[];
  static const char STATIC_BUFFER_LENGTH = 33;


  union U {
    df_pointer    as_pointer;
    df_number     as_number;
    df_string*    as_string;
    df_date       as_date;
    df_category   as_category;

    ~U() {}
    U() {}
  } data;



  inline void release() {
    if constexpr (std::is_same_v<T, df_string>) {
      if (data.as_string) {
        delete data.as_string;
      }
    }
  }


  inline void from_move(df_object& src) {
    if constexpr (std::is_pointer_v<T> || std::is_same_v<T, df_string>) {
      data.as_pointer = src.data.as_pointer;
      src.data.as_pointer = NULL;
      return;
    }
    // normal
    data = src.data;
  }

  inline void from_copy(const df_object& src) {
    // string
    if constexpr (std::is_same_v<T, df_string>) {
      data.as_string = src.data.as_string ? new df_string(*src.data.as_string) : NULL;
      return;
    }
    // normal
    data = src.data;
  }


public:
  inline ~df_object() {
    release();
  }


  // == create ==

  inline df_object(const T& src) {
    df_debug1("create1 df_object (%d)", df_get_type_v<T>);

    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = src;
      return;
    }
    if constexpr (std::is_same_v<T, df_string>) {
      data.as_string = src ? new df_string(src) : NULL;
      return;
    }
    if constexpr (std::is_same_v<T, df_date>) {
      data.as_date = src;
      return;
    }
    data.as_number = (df_number)src;
  }
  
  inline df_object(const char* src) {
    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = (void*)src;
      return;
    }
    if constexpr (std::is_same_v<T, df_string>) {
      data.as_string = src ? new df_string(src) : NULL;
      return;
    }
    if constexpr (std::is_same_v<T, df_date>) {
      data.as_date = df_date(src);
      return;
    }

    // == number ==
    if (src == NULL) {
      data.as_number = DF_NULL_NUMBER;
      return;
    }

    sscanf(src, "%lf", &data.as_number);
  }


  // is null?
  inline df_boolean is_null() const {
    if constexpr (std::is_same_v<T, df_string> || std::is_pointer_v<T>) {
      return data.as_pointer == DF_NULL_POINTER;
    }
    if constexpr (std::is_same_v<T, df_date>) {
      return data.as_date == DF_NULL_DATE;
    }
    if constexpr (std::is_same_v<T, df_category>) {
      return data.as_date == DF_NULL_CATEGORY;
    }
    return isnan(data.as_number);
  }


  // == get / set ==

  inline operator T() const {
    if constexpr (std::is_pointer_v<T>) {
      return (T)data.as_pointer;
    }
    else if constexpr (std::is_same_v<T, df_string>) {
      return *data.as_string;
    }
    else if constexpr (std::is_same_v<T, df_date>) {
      return data.as_date;
    }
    else {
      return data.as_number;
    }
  }



  inline df_object& operator=(const T& src) {
    if constexpr (std::is_same_v<T, df_string>) {
      release();
      data.as_string = src ? new df_string(src) : NULL;
      return *this;
    }
    data = src;
    return *this;
  }



  // == move ==

  inline df_object(df_object&& src) {
    from_move(src);
  }

  inline df_object& operator=(df_object&& src) {
    release();
    from_move(src);
    return *this;
  }


  // == copy ==

  inline df_object(const df_object& src) {
    df_debug1("copy1 df_object (%d)", df_get_type_v<T>);
    from_copy(src);
  }

  inline df_object& operator=(const df_object& src) {
    df_debug1("copy2 df_object (%d)", df_get_type_v<T>);
    release();
    from_copy(src);
    return *this;
  }


  // == c_str() ==

  inline const char* c_str(int type = df_get_type_v<T>, const char* fmtFloat = DF_DEFAULT_FLOAT_FORMAT, char* buffer = DF_STATIC_BUFFER) const {
    if (is_null()) {
      return "null";
    }

    switch (type) {
      case DF_TYPE_POINTER:
        snprintf(buffer, DF_STATIC_BUFFER_LENGTH, "%p", data.as_pointer);
        return buffer;
      case DF_TYPE_BOOLEAN:
        return data.as_number ? "true" : "false";
      case DF_TYPE_STRING:
        return data.as_string->c_str();
      case DF_TYPE_NUMBER:
        snprintf(buffer, DF_STATIC_BUFFER_LENGTH, fmtFloat, data.as_number);
        return buffer;
      case DF_TYPE_DATE:
        return data.as_date.c_str();
      case DF_TYPE_CATEGORY:
        snprintf(buffer, DF_STATIC_BUFFER_LENGTH, "category(%ld)", data.as_category.category);
        return buffer;
      default:
        return "unknown-type";
    }
  }
};




#endif // _DF_OBJECT_HPP_