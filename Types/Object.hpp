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




typedef std::string df_string;



df_string df_repr(const df_string& s) {
  df_string out = "\"";
  for (char c : s) {
    switch (c) {
      case '\n': out += "\\n"; break;
      case '\t': out += "\\t"; break;
      case '\r': out += "\\r"; break;
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      default:
        if (isprint(c)) {
          out += c;
        }

        char buf[5];
        snprintf(buf, 5, "\\x%02x", c);
        out += buf;
    }
  }
  out += "\"";
  return out;
}





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
    static constexpr int value =
      std::is_pointer_v<T> ? DF_TYPE_POINTER
      : std::is_same_v<T, bool> ? DF_TYPE_BOOLEAN
      : std::is_same_v<T, df_string> ? DF_TYPE_STRING
      : std::is_same_v<T, df_date> ? DF_TYPE_DATE
      : DF_TYPE_NUMBER;
};

template<typename T>
inline constexpr bool df_get_type_v = df_get_type<T>::value;



// template<typename T>
// df_type df_get_type() {
//   if constexpr (std::is_pointer_v<T>) {
//     return DF_TYPE_POINTER;
//   }
//   if constexpr (std::is_same_v<T, bool>) {
//     return DF_TYPE_BOOLEAN;
//   }
//   if constexpr (std::is_same_v<T, df_string>) {
//     return DF_TYPE_STRING;
//   }
//   if constexpr (std::is_same_v<T, df_date>) {
//     return DF_TYPE_DATE;
//   }
//   return DF_TYPE_CATEGORY;
// }





template<typename T> class df_object_chunk;


template<typename T = DF_DEFAULT_TYPE>
class df_object {
  static_assert(sizeof(T) <= 8, "T size could not over 8 bytes!");
  static_assert(!std::is_const_v<T>, "not allowed to use const value");

  friend class df_object_chunk<T>;
  friend int main(int argc, char** argv);

  static char STATIC_BUFFER[];
  static const char STATIC_BUFFER_LENGTH = 33;


  union U {
    void* as_pointer;

    double as_number;
    df_string* as_string;
    df_date as_date;

    ~U() {}
    U() {}
  } data;



  inline void release() {
    if constexpr (std::is_same_v<T, df_string>) {
      delete data.as_string;
    }
  }

  inline void from_string(const char* src) {
    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = (void*)src;
      return;
    }
    if constexpr (std::is_same_v<T, df_string>) {
      data.as_string = new df_string(src);
      return;
    }
    if constexpr (std::is_same_v<T, df_date>) {
      data.as_date = df_date(src);
      return;
    }
    throw df_exception("only STRING or DATE can use df_object(const char*)");
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
      data.as_string = new std::string(*src.data.as_string);
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
    if constexpr (std::is_same_v<T, df_string>) {
      data.as_string = new df_string(src);
      return;
    }
    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = src;
      return;
    }
    data.as_number = (double)src;
  }



  // == from string ==

  inline df_object(const char* src) noexcept(false) {
    from_string(src);
  }


  inline df_object& operator=(const char* src) {
    release();
    from_string(src);
    return *this;
  }


  // is null?
  inline bool isNull() {
    if constexpr (std::is_same_v<T, df_string> || std::is_pointer_v<T>) {
      return data.as_pointer == NULL;
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
      data.as_string = new std::string(src);
      return *this;
    }
    data = src;
    return *this;
  }



  // == move ==

  inline df_object(df_object&& src) {
    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = src.data.as_pointer;
      src.data.as_pointer = NULL;
      return;
    }
    data = src.data;
  }

  inline df_object& operator=(df_object&& src) {
    if constexpr (std::is_pointer_v<T>) {
      data.as_pointer = src.data.as_pointer;
      src.data.as_pointer = NULL;
      return *this;
    }
    if constexpr (std::is_same_v<T, df_string>) {
      release();
      data.as_string = new std::string(*src.data.as_string);
      data.as_string = src.data.as_string;
      src.data.as_string = NULL;
      return *this;
    }
    data = src.data;
    return *this;
  }


  // == copy ==

  inline df_object(const df_object& src) {
    from_copy(src);
  }

  inline df_object& operator=(const df_object& src) {
    if constexpr (std::is_same_v<T, df_string>) {
      release();
    }
    from_copy(src);
    return *this;
  }


  // == c_str() ==

  inline const char* c_str(char* buffer = STATIC_BUFFER, const char* fmtFloat = "%g") const {
    // pointer
    if constexpr (std::is_pointer_v<T>) {
      snprintf(buffer, STATIC_BUFFER_LENGTH - 1, "%p", data.as_pointer);
      return buffer;
    }
    // boolean
    if constexpr (std::is_same_v<T, bool>) {
      return data.as_number ? "true" : "false";
    }
    // string
    if constexpr (std::is_same_v<T, df_string>) {
      return data.as_string->c_str();
    }
    // date
    if constexpr (std::is_same_v<T, df_date>) {
      return data.as_date.toString();
    }
    // number
    snprintf(buffer, STATIC_BUFFER_LENGTH - 1, fmtFloat, data.as_number);
    return buffer;
  }
};


template<typename T> char df_object<T>::STATIC_BUFFER[df_object<T>::STATIC_BUFFER_LENGTH];




#endif // _DF_OBJECT_HPP_