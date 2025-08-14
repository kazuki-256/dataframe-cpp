#ifndef _DF_EXCEPTION_HPP_
#define _DF_EXCEPTION_HPP_

#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>



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