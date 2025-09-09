#pragma once

#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>





// df_value_t level debug
#if DF_DEBUG_LEVEL <= 1
#define df_debug1(fmt, ...) fprintf(stderr, "debug1: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug1(...)
#endif

// df_object_t level debug
#if DF_DEBUG_LEVEL <= 2
#define df_debug2(fmt, ...) fprintf(stderr, "debug2: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug2(...)
#endif

// df_mem_block_t level debug
#if DF_DEBUG_LEVEL <= 3
#define df_debug3(fmt, ...) fprintf(stderr, "debug3: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug3(...)
#endif

// df_column_t level debug
#if DF_DEBUG_LEVEL <= 4
#define df_debug4(fmt, ...) fprintf(stderr, "debug4: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug4(...)
#endif

// df_dataframe_t level debug
#if DF_DEBUG_LEVEL <= 5
#define df_debug5(fmt, ...) fprintf(stderr, "debug5: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug5(...)
#endif

// df_query_t level debug
#if DF_DEBUG_LEVEL <= 6
#define df_debug6(fmt, ...) fprintf(stderr, "debug6: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug6(...)
#endif

// debug
#if DF_DEBUG_LEVEL <= 7
#define df_debug7(fmt, ...) fprintf(stderr, "debug7: " __FILE__ "-%d: %s: " fmt "\n", __LINE__ , __func__, ##__VA_ARGS__);
#else
#define df_debug7(...)
#endif



class df_exception_t : public std::exception {
protected:
  char* msg = NULL;

  df_exception_t() {};
public:
  df_exception_t(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t len = strlen(fmt);
    msg = (char*)malloc(len + 378);
    vsnprintf(msg, len + 377, fmt, args);

    va_end(args);
  }

  virtual ~df_exception_t() {
    if (msg != NULL) free(msg);
  }

  const char* what() const noexcept override {
    return msg;
  }
};


class df_exception_out_of_index : df_exception_t {
  const char* what() const noexcept override {
    return "out of index";
  }
};

class df_exception_interval_couldnot_be_0 : df_exception_t {
  const char* what() const noexcept override {
    return "interval couldn't be 0";
  }
};

class df_exception_endless_range : df_exception_t {
  const char* what() const noexcept override {
    return "endless range";
  }
};

class df_exception_not_enough_memory : df_exception_t {
  const char* what() const noexcept override {
    return "not enough memory";
  }
};




#define DF_RETURN_IF_NULL(mem, ret)\
  if (mem == NULL) {\
    fprintf(stderr, "%s-%d: " #mem " is a null pointer!", __FILE__, __LINE__);\
    return ret;\
  }

