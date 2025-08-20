#ifndef _DF_EXCEPTION_HPP_
#define _DF_EXCEPTION_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif


#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>





#if DF_DEBUG_LEVEL <= 1
// DfObject level debug
#define df_debug1(fmt, ...) fprintf(stderr, "debug1: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug1(...)
#endif

#if DF_DEBUG_LEVEL <= 2
// DfObjectChunk level debug
#define df_debug2(fmt, ...) fprintf(stderr, "debug2: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug2(...)
#endif

#if DF_DEBUG_LEVEL <= 3
// DfColumn level debug
#define df_debug3(fmt, ...) fprintf(stderr, "debug3: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug3(...)
#endif

#if DF_DEBUG_LEVEL <= 4
// DfDataFrame level debug
#define df_debug4(fmt, ...) fprintf(stderr, "debug4: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug4(...)
#endif

#if DF_DEBUG_LEVEL <= 5
// DfProcess level debug
#define df_debug5(fmt, ...) fprintf(stderr, "debug5: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug5(...)
#endif

#if DF_DEBUG_LEVEL <= 6
#define df_debug6(fmt, ...) fprintf(stderr, "debug6: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug6(...)
#endif



class df_exception_t : public std::exception {
  char* msg;
public:
  df_exception_t(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t len = strlen(fmt);
    msg = (char*)malloc(len + 378);
    vsnprintf(msg, len + 377, fmt, args);

    va_end(args);
  }

  ~df_exception_t() {
    free(msg);
  }

  const char* what() const noexcept override {
    return msg;
  }
};


class df_exception_out_of_index : public std::exception {
  const char* what() const noexcept override {
    return "out of index";
  }
};


#define df_error_if_null_pointer(mem, ret)\
  if (mem == NULL) {\
    fprintf(stderr, "%s-%d: null pointer!", __FILE__, __LINE__);\
    return ret;\
  }



#endif // _DF_EXCEPTION_HPP_