#ifndef _DF_CONFIG_HPP_
#define _DF_CONFIG_HPP_

const char DF_VERSION[] = "beta 1.0.0";

// includes
#ifndef _GLIBCXX_VECTOR
#include <vector>
#endif
#ifndef _GLIBCXX_LIST
#include <list>
#endif
#ifndef _GLIBCXX_STRING
#include <string>
#endif
#ifndef _GLIBCXX_OSTREAM
#include <ostream>
#endif


#ifndef _TIME_H_
#include <time.h>
#endif
#ifndef _STRING_H_
#include <string.h>
#endif
#ifndef _STDIO_H_
#include <stdio.h>
#endif
#ifndef _MATH_H_
#include <math.h>
#endif


#ifndef _DF_TYPE_EXECEPTION_HPP_
#include "types/exception.hpp"
#endif





#if __cplusplus < 201703L
  #error "Azuki's DataFrame requires C++17 or higher. Please enable -std=c++17 or later."
#endif



// == types ==

class df_exception_t;

class df_date_t;

class df_object_t;
class df_column_t;
class df_dataframe_t;

class df_query_t;



typedef const char* (*df_byte_cstr_callback)(void* data, char* buffer, size_t buffer_size);



// == enums ==

constexpr int DF_SIZE_SHIFT = 0x3f;
constexpr int DF_TYPE_SHIFT = 6;
constexpr int DF_TYPE_COUNT = 15;

constexpr int DF_MAX_TYPE_SIZE = 32;


typedef enum df_type_t {
  DF_POINTER = 0 << DF_TYPE_SHIFT | 8,
  DF_NULL = 1 << DF_TYPE_SHIFT | 1,

  DF_UINT8 = 2 << DF_TYPE_SHIFT | 1,
  DF_INT16 = 3 << DF_TYPE_SHIFT | 2,
  DF_INT32 = 4 << DF_TYPE_SHIFT | 4,
  DF_INT64 = 5 << DF_TYPE_SHIFT | 8,

  DF_FLOAT32 = 6 << DF_TYPE_SHIFT | 4,
  DF_FLOAT64 = 7 << DF_TYPE_SHIFT | 8,

  DF_TEXT = 8 << DF_TYPE_SHIFT | 32,
  DF_CATEGORY = 9 << DF_TYPE_SHIFT | 2,

  DF_DATE = 10 << DF_TYPE_SHIFT | 8,
  DF_TIME = 11 << DF_TYPE_SHIFT | 8,
  DF_DATETIME = 12 << DF_TYPE_SHIFT | 8,
  DF_INTERVAL = 13 << DF_TYPE_SHIFT | 24,

  DF_BOOL = 14 << DF_TYPE_SHIFT | 1,
} df_type_t;


// template<typename T>
// struct df_type_get_type {
//   df_type_t type =
//     std::is_pointer_v<T> ? DF_POINTER

//     : std::is_same_v<T, uint8_t> ? DF_UINT8
//     : std::is_same_v<T, short> ? DF_INT16
//     : std::is_same_v<T, int> ? DF_INT32
//     : std::is_same_v<T, long> ? DF_INT64

//     : std::is_same_v<T, float> ? DF_FLOAT32
//     : std::is_same_v<T, double> ? DF_FLOAT64

//     : std::is_same_v<T, std::string> ? DF_TEXT
//     : std::is_same_v<T, const char*> ? DF_TEXT

//     : std::is_same_v<T, date_t> ? DF_DATETIME

//     : std::is_same_v<T, bool> ? DF_BOOL
//     : DF_NULL;
// };

template<typename T>
df_type_t df_type_get_type = std::is_pointer_v<T> ? DF_POINTER

    : std::is_same_v<T, uint8_t> ? DF_UINT8
    : std::is_same_v<T, short> ? DF_INT16
    : std::is_same_v<T, int> ? DF_INT32
    : std::is_same_v<T, long> ? DF_INT64

    : std::is_same_v<T, float> ? DF_FLOAT32
    : std::is_same_v<T, double> ? DF_FLOAT64

    : std::is_same_v<T, std::string> ? DF_TEXT
    : std::is_same_v<T, const char*> ? DF_TEXT

    : std::is_same_v<T, df_date_t> ? DF_DATETIME

    : std::is_same_v<T, bool> ? DF_BOOL
    : DF_NULL;



constexpr inline int df_type_get_number(df_type_t type) {
  return type >> DF_TYPE_SHIFT;
}

constexpr inline int df_type_get_size(df_type_t type) {
  return type & DF_SIZE_SHIFT;
}

inline const char* df_type_get_string(df_type_t type) {
  static const char* TYPE_NAME_LIST[DF_TYPE_COUNT] = {
    "POINTER", "NULL",
    "UINT8", "INT16", "INT32", "INT64",
    "FLOAT32", "FLOAT64",
    "TEXT", "CATEGORY",
    "DATE", "TIME", "DATETIME", "INTERVAL",
    "BOOL"
  };

  int type_number = df_type_get_number(type);
  return type_number < DF_TYPE_COUNT ? TYPE_NAME_LIST[type_number] : "INVALID_TYPE";
}





#define DF_MAX(A, B) ((A) > (B) ? (A) : (B))
#define DF_MIN(A, B) ((A) > (B) ? (A) : (B))




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
df_string df_repr_string(const df_string& s) {
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