#ifndef _DF_CONFIG_HPP_
#define _DF_CONFIG_HPP_

#define DF_VERSION "beta 1.0.0"

#if __cplusplus < 201703L
  #error "Azuki's DataFrame requires C++17 or higher. Please enable -std=c++17 or later."
#endif




#include <vector>









#define DF_DEFAULT_TYPE df_number

char DF_STATIC_BUFFER[257];
#define DF_STATIC_BUFFER_LENGTH 256


#define DF_DEFAULT_FLOAT_FORMAT "%g"
#define DF_DEFAULT_RETURN_POINTER (int*)DF_STATIC_BUFFER

#define DF_CHUNK_GROWTH_FACTOR 2


#define DF_NULL_DATE INT64_MIN
#define DF_NULL_CATEGORY 0
#define DF_NULL_STRING NULL
#define DF_NULL_POINTER NULL
#define DF_NULL_NUMBER NAN


#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif








template<typename T> class df_object;
template<typename T> class df_object_chunk;
template<typename T> class df_column;
class df_data_frame;
class df_process;



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