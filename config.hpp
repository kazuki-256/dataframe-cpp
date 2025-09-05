#pragma once

const char DF_VERSION[] = "beta 1.0.0";

// == includes ==

#include <string>
#include <iostream>

#include <string.h>





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
