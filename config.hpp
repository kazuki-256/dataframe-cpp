#ifndef _DF_CONFIG_HPP_
#define _DF_CONFIG_HPP_

const char DF_VERSION[] = "beta 1.0.0";

// includes
#include "includes.hpp"


#if __cplusplus < 201703L
  #error "Azuki's DataFrame requires C++17 or higher. Please enable -std=c++17 or later."
#endif



// == enums ==

constexpr int DF_TYPE_CATEGORY_SHIFT = 10;
constexpr int DF_TYPE_TYPE_SHIFT = 6;
constexpr int DF_TYPE_SIZE_SHIFT = 0;

constexpr int DF_TYPE_CATEGORY_MASK = 0xf << DF_TYPE_CATEGORY_SHIFT;
constexpr int DF_TYPE_TYPE_MASK = 0xf << DF_TYPE_TYPE_SHIFT;
constexpr int DF_TYPE_SIZE_MASK = 0x3f;


enum {
  DF_TYPE_CATEGORY_SPECIAL = 0 << DF_TYPE_CATEGORY_SHIFT,
  DF_TYPE_CATEGORY_INT = 1 << DF_TYPE_CATEGORY_SHIFT,
  DF_TYPE_CATEGORY_FLOAT = 2 << DF_TYPE_CATEGORY_SHIFT,
  DF_TYPE_CATEGORY_TEXT = 3 << DF_TYPE_CATEGORY_SHIFT,
  DF_TYPE_CATEGORY_DATE = 4 << DF_TYPE_CATEGORY_SHIFT,
  DF_TYPE_CATEGORY_BOOLEAN = 5 << DF_TYPE_CATEGORY_SHIFT,
};


typedef enum df_type_t {
  DF_UNDEFINED = DF_TYPE_CATEGORY_SPECIAL | 0 | 1,
  DF_POINTER = DF_TYPE_CATEGORY_SPECIAL | 1 | 8,
  DF_NULL = DF_TYPE_CATEGORY_SPECIAL | 2 | 1,

  DF_UINT8 = DF_TYPE_CATEGORY_INT | 0 << DF_TYPE_TYPE_SHIFT | 1,
  DF_INT16 = DF_TYPE_CATEGORY_INT | 1 << DF_TYPE_TYPE_SHIFT | 2,
  DF_INT32 = DF_TYPE_CATEGORY_INT | 2 << DF_TYPE_TYPE_SHIFT | 4,
  DF_INT64 = DF_TYPE_CATEGORY_INT | 3 << DF_TYPE_TYPE_SHIFT | 8,

  DF_FLOAT32 = DF_TYPE_CATEGORY_FLOAT | 0 << DF_TYPE_TYPE_SHIFT | 4,
  DF_FLOAT64 = DF_TYPE_CATEGORY_FLOAT | 1 << DF_TYPE_TYPE_SHIFT | 8,

  DF_TEXT = DF_TYPE_CATEGORY_TEXT | 0 << DF_TYPE_TYPE_SHIFT | 32,
  DF_CATEGORY = DF_TYPE_CATEGORY_TEXT | 1 << DF_TYPE_TYPE_SHIFT | 2,

  DF_DATE = DF_TYPE_CATEGORY_DATE | 0 << DF_TYPE_TYPE_SHIFT | 8,
  DF_TIME = DF_TYPE_CATEGORY_DATE | 1 << DF_TYPE_TYPE_SHIFT | 8,
  DF_DATETIME = DF_TYPE_CATEGORY_DATE | 2 << DF_TYPE_TYPE_SHIFT | 8,
  DF_INTERVAL = DF_TYPE_CATEGORY_DATE | 3 << DF_TYPE_TYPE_SHIFT | 24,

  DF_BOOLEAN = DF_TYPE_CATEGORY_BOOLEAN | 0 << DF_TYPE_TYPE_SHIFT | 1,
} type;


constexpr inline int df_get_type_size(df_type_t type) {
  return type & DF_TYPE_SIZE_MASK;
}

constexpr inline int df_get_type_category(df_type_t type) {
  return type & DF_TYPE_CATEGORY_MASK;
}

inline const char* df_get_type_string(df_type_t type) {
  switch (df_get_type_category(type)) {
    case DF_TYPE_CATEGORY_SPECIAL:
      switch (type) {
        case DF_UNDEFINED:    return "UNDEFINED";
        case DF_POINTER:      return "POINTER";
        case DF_NULL:         return "NULL";
      }
      break;
    case DF_TYPE_CATEGORY_INT:
      switch (type) {
        case DF_UINT8:  return "UINT8";
        case DF_INT16:  return "INT16";
        case DF_INT32:  return "INT32";
        case DF_INT64:  return "INT64";
      }
      break;
    case DF_TYPE_CATEGORY_FLOAT:
      switch (type) {
        case DF_FLOAT32: return "FLOAT32";
        case DF_FLOAT64: return "FLOAT64";
      }
      break;
    case DF_TYPE_CATEGORY_TEXT:
      switch (type) {
        case DF_TEXT:     return "TEXT";
        case DF_CATEGORY: return "CATEGORY";
      }
      break;
    case DF_TYPE_CATEGORY_DATE:
      switch (type) {
        case DF_DATE:  return "DATE";
        case DF_TIME:  return "TIME";
        case DF_DATETIME:  return "DATETIME";
      }
      break;
    case DF_TYPE_CATEGORY_BOOLEAN:
      return "BOOLEAN";
  }
  return "INVALID_TYPE";
}



// == types ==

class df_exception_t;

class df_category_t;
class df_date_t;


template<df_type_t TYPE> class df_raw_t;
template<df_type_t TYPE> class df_raw_block_t;
template<df_type_t TYPE> class df_column_t;

class df_object_t;
class df_dataframe_t;
class df_query_t;



typedef const char* (*df_c_str_raw_callback)(df_raw_t<DF_UNDEFINED>& raw, char* buffer, size_t buffer_size);





template<typename T>
T& DF_MAX(T& a, T& b) {
  return a > b ? a : b;
}

template<typename T>
T& DF_MIN(T& a, T& b) {
  return a < b ? a : b;
}





// ==== constants ====

// == value information ==

constexpr uint8_t DF_NULL_UINT8 = 255;
constexpr uint8_t DF_MIN_UINT8 = 0;
constexpr uint8_t DF_MAX_UINT8 = 254;

constexpr short DF_NULL_INT16 = -32768;
constexpr short DF_MIN_INT16 = -32767;
constexpr short DF_MAX_INT16 = 32767;

constexpr int DF_NULL_INT32 = -2147483648;
constexpr int DF_MIN_INT32 = -2147483647;
constexpr int DF_MAX_INT32 = 2147483647;

constexpr long DF_NULL_INT64 = 0x8000000000000000;
constexpr long DF_MIN_INT64 = 0x8000000000000001;
constexpr long DF_MAX_INT64 = 0x7fffffffffffffff;

constexpr float DF_NULL_FLOAT32 = NAN;
constexpr double DF_NULL_FLOAT64 = NAN;

constexpr time_t DF_NULL_DATE = DF_NULL_INT64;
constexpr time_t DF_MIN_DATE = DF_MIN_INT64;
constexpr time_t DF_MAX_DATE = DF_MAX_INT64;

constexpr uint8_t DF_NULL_BOOLEAN = 255;





// == other ==

constexpr int DF_STATIC_BUFFER_LENGTH = 128;
char DF_STATIC_BUFFER[DF_STATIC_BUFFER_LENGTH + 1];



constexpr char DF_INT_FORMAT[] = "%ld";
constexpr char DF_FLOAT_FORMAT[] = "%lg";

constexpr char DF_DATE_FORMAT[] = "%Y-%m-%d";
constexpr char DF_TIME_FORMAT[] = "%H:%M:%S";
constexpr char DF_DATETIME_FORMAT[] = "%Y-%m-%d %H:%M:%S";






/*
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
  */


#endif // _DF_CONFIG_HPP_