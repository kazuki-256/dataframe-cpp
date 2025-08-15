#ifndef _DF_EXCEPTION_HPP_
#define _DF_EXCEPTION_HPP_

#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>





#if DF_DEBUG_LEVEL <= 1
// DfObject level debug
#define DfDebug1(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug1(...)
#endif

#if DF_DEBUG_LEVEL <= 2
// DfObjectChunk level debug
#define DfDebug2(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug2(...)
#endif

#if DF_DEBUG_LEVEL <= 3
// DfColumn level debug
#define DfDebug3(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug3(...)
#endif

#if DF_DEBUG_LEVEL <= 4
// DfDataFrame level debug
#define DfDebug4(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug4(...)
#endif

#if DF_DEBUG_LEVEL <= 5
// DfProcess level debug
#define DfDebug5(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug5(...)
#endif

#if DF_DEBUG_LEVEL <= 6
#define DfDebug6(fmt, ...) fprintf(stderr, "debug %s-%d: " fmt "\n", __FILE__, __LINE__ ,##__VA_ARGS__);
#else
#define DfDebug6(...)
#endif



class DfException : public std::exception {
  char* msg;
public:
  DfException(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t len = strlen(fmt);
    msg = (char*)malloc(len + 378);
    vsnprintf(msg, len + 377, fmt, args);

    va_end(args);
  }

  ~DfException() {
    free(msg);
  }

  const char* what() const noexcept override {
    return msg;
  }
};


class DfExceptionOutOfIndex : public std::exception {
  const char* what() const noexcept override {
    return "out of index";
  }
};

#endif // _DF_EXCEPTION_HPP_