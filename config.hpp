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
#ifndef _GLIBCXX_OPTIONAL
#include <optional>
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

typedef union df_value_t df_value_t;

class df_exception_t;

class df_date_t;
class df_null_t;

class df_object_t;
class df_row_t;

class df_column_t;
typedef std::pair<std::string, df_column_t> df_named_column_t;

class df_dataframe_t;

class df_query_t;

typedef std::optional<std::string> df_string_t;



typedef const char* (*df_byte_cstr_callback)(void* data, char* buffer, size_t buffer_size);



// == enums ==

constexpr int DF_SIZE_SHIFT = 0x3f;
constexpr int DF_TYPE_SHIFT = 6;
constexpr int DF_TYPE_COUNT = 15;

constexpr int DF_MAX_TYPE_SIZE = 32;


typedef enum df_type_id_t {
  DF_TYPEID_POINTER,
  DF_TYPEID_NULL,

  DF_TYPEID_UINT8,
  DF_TYPEID_INT16,
  DF_TYPEID_INT32,
  DF_TYPEID_INT64,

  DF_TYPEID_FLOAT32,
  DF_TYPEID_FLOAT64,
  
  DF_TYPEID_TEXT,
  DF_TYPEID_CATEGORY,

  DF_TYPEID_DATE,
  DF_TYPEID_TIME,
  DF_TYPEID_DATETIME,
  DF_TYPEID_INTERVAL,

  DF_TYPEID_BOOL,
} df_type_id_t;


#define DF_TYPESIZE_POINTER 8
#define DF_TYPESIZE_NULL 1

#define DF_TYPESIZE_UINT8 1
#define DF_TYPESIZE_INT16 2
#define DF_TYPESIZE_INT32 4
#define DF_TYPESIZE_INT64 8

#define DF_TYPESIZE_FLOAT32 4
#define DF_TYPESIZE_FLOAT64 8

#define DF_TYPESIZE_TEXT 32
#define DF_TYPESIZE_CATEGORY 4

#define DF_TYPESIZE_DATETIME 8
#define DF_TYPESIZE_INTERVAL 24

#define DF_TYPESIZE_BOOL 1



typedef enum df_type_t {
  DF_TYPE_POINTER = DF_TYPEID_POINTER << DF_TYPE_SHIFT | DF_TYPESIZE_POINTER,    // void*
  DF_TYPE_NULL = DF_TYPEID_NULL << DF_TYPE_SHIFT | DF_TYPESIZE_NULL,          // df_null_t

  DF_TYPE_UINT8 = DF_TYPEID_UINT8 << DF_TYPE_SHIFT | DF_TYPESIZE_UINT8,        // uint8_t
  DF_TYPE_INT16 = DF_TYPEID_INT16 << DF_TYPE_SHIFT | DF_TYPESIZE_INT16,        // short
  DF_TYPE_INT32 = DF_TYPEID_INT32 << DF_TYPE_SHIFT | DF_TYPESIZE_INT32,        // int
  DF_TYPE_INT64 = DF_TYPEID_INT64 << DF_TYPE_SHIFT | DF_TYPESIZE_INT64,        // long

  DF_TYPE_FLOAT32 = DF_TYPEID_FLOAT32 << DF_TYPE_SHIFT | DF_TYPESIZE_FLOAT32,    // float
  DF_TYPE_FLOAT64 = DF_TYPEID_FLOAT64 << DF_TYPE_SHIFT | DF_TYPESIZE_FLOAT64,    // double

  DF_TYPE_TEXT = DF_TYPEID_TEXT << DF_TYPE_SHIFT | DF_TYPESIZE_TEXT,         // std::optional<std::string>
  DF_TYPE_CATEGORY = DF_TYPEID_CATEGORY << DF_TYPE_SHIFT | DF_TYPESIZE_CATEGORY,  // int

  DF_TYPE_DATE = DF_TYPEID_DATE << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,          // df_date_t
  DF_TYPE_TIME = DF_TYPEID_TIME << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,          // df_date_t
  DF_TYPE_DATETIME = DF_TYPEID_DATETIME << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,  // df_date_t
  DF_TYPE_INTERVAL = DF_TYPEID_INTERVAL << DF_TYPE_SHIFT | DF_TYPESIZE_INTERVAL, // df_interval_t

  DF_TYPE_BOOL = DF_TYPEID_BOOL << DF_TYPE_SHIFT | DF_TYPESIZE_BOOL,          // bool

  // == alias ==

  DF_TYPE_SHORT = DF_TYPE_INT16,
  DF_TYPE_INT = DF_TYPE_INT32,
  DF_TYPE_LONG = DF_TYPE_INT64,

  DF_TYPE_STRING = DF_TYPE_TEXT,
} df_type_t;


template<typename T>
df_type_t df_type_get_type =
    std::is_same_v<T, df_string_t> ? DF_TYPE_TEXT
    : std::is_same_v<T, std::string> ? DF_TYPE_TEXT
    : std::is_same_v<T, const char*> ? DF_TYPE_TEXT

    : std::is_pointer_v<T> ? DF_TYPE_POINTER

    : std::is_same_v<T, uint8_t> ? DF_TYPE_UINT8
    : std::is_same_v<T, short> ? DF_TYPE_INT16
    : std::is_same_v<T, int> ? DF_TYPE_INT32
    : std::is_same_v<T, long> ? DF_TYPE_INT64

    : std::is_same_v<T, float> ? DF_TYPE_FLOAT32
    : std::is_same_v<T, double> ? DF_TYPE_FLOAT64

    : std::is_same_v<T, df_date_t> ? DF_TYPE_DATETIME

    : std::is_same_v<T, bool> ? DF_TYPE_BOOL
    : DF_TYPE_NULL;




constexpr inline bool df_type_is_struct(df_type_t type) {
  return type == DF_TYPE_TEXT;
}

constexpr inline bool df_type_is_struct(df_type_id_t type) {
  return type == DF_TYPEID_TEXT;
}

constexpr inline int df_type_get_typeid(df_type_t type) {
  return type >> DF_TYPE_SHIFT;
}

constexpr inline bool df_type_is_number(df_type_t type) {
  int type_id = df_type_get_typeid(type);
  return (type_id >= DF_TYPEID_UINT8 && type_id <= DF_TYPEID_FLOAT64) || type == DF_TYPE_BOOL;
}

constexpr inline int df_type_get_size(df_type_t type) {
  return type & DF_SIZE_SHIFT;
}

const char* df_typeid_get_string(int type_id) {
  constexpr static const char* TYPE_NAMES[DF_TYPE_COUNT] = {
    "POINTER", "NULL",
    "UINT8", "SHORT", "INT", "LONG",
    "FLOAT", "DOUBLE",
    "TEXT", "CATEGORY",
    "DATE", "TIME", "DATETIME", "INTERVAL",
    "BOOL"
  };

  return type_id < DF_TYPE_COUNT ? TYPE_NAMES[type_id] : "INVALID_TYPE";
}

inline const char* df_type_get_string(df_type_t type) {
  return df_typeid_get_string(df_type_get_typeid(type));
}





#define DF_MAX(A, B) ((A) > (B) ? (A) : (B))
#define DF_MIN(A, B) ((A) > (B) ? (A) : (B))




// ==== constants ====

// == value information ==

#define DF_NULL_POINTER 0x0

#define DF_NULL_UINT8 0xff
#define DF_MIN_UINT8  0x0
#define DF_MAX_UINT8  0xfe

#define DF_NULL_INT16 0x8000
#define DF_MIN_INT16  0x8001
#define DF_MAX_INT16  0x7fff

#define DF_NULL_INT32 0x80000000
#define DF_MIN_INT32  0x80000001
#define DF_MAX_INT32  0x7fffffff

#define DF_NULL_INT64 0x8000000000000000
#define DF_MIN_INT64  0x8000000000000001
#define DF_MAX_INT64  0x7fffffffffffffff

#define DF_NULL_FLOAT32 0x7FC00000            // ⚠️ Represents a quiet NaN (float) encoded as an int32. Used for forced memory writes. Not safe for arithmetic or logical operations.
#define DF_NULL_FLOAT64 0x7ff8000000000000    // ⚠️ Represents a quiet NaN (double) encoded as an int64. Used for forced memory writes. Not safe for arithmetic or logical operations.

#define DF_NULL_CATEGORY DF_NULL_INT32

#define DF_NULL_DATETIME DF_NULL_INT64

#define DF_NULL_BOOL DF_NULL_UINT8





// == other ==

constexpr int DF_STATIC_BUFFER_LENGTH = 128;
char DF_STATIC_BUFFER[DF_STATIC_BUFFER_LENGTH + 1];



constexpr char DF_INT32_FORMAT[] = "%d";
constexpr char DF_INT64_FORMAT[] = "%ld";

constexpr char DF_FLOAT32_FORMAT[] = "%g";
constexpr char DF_FLOAT64_FORMAT[] = "%lg";

constexpr char DF_DATE_FORMAT[] = "%Y-%m-%d";
constexpr char DF_TIME_FORMAT[] = "%H:%M:%S";
constexpr char DF_DATETIME_FORMAT[] = "%Y-%m-%d %H:%M:%S";



constexpr char DF_DEFAULT_COLUMN_NAME[] = "COLUMN";

#define DF_DEFAULT_COLUMN_SMALL_START_CAPACITY 4096
#define DF_DEFAULT_COLUMN_BIG_START_CAPACITY 32768





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