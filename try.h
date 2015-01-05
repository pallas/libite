#ifndef TRY_H
#define TRY_H

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include <errno.h>

#define TRY(f, ...) ({ \
  typeof (f(__VA_ARGS__)) _r = f(__VA_ARGS__); \
  if (_r < 0) perror(#f), throw std::runtime_error(strerror(errno)); \
  _r; \
})

#define TRY_ERR(e, f, ...) ({ \
  typeof (f(__VA_ARGS__)) _r = f(__VA_ARGS__); \
  if (_r < 0 && errno != e) perror(#f), throw std::runtime_error(strerror(errno)); \
  _r; \
})

#define TRY_PTR(f, ...) ({ \
    typeof (f(__VA_ARGS__)) _r = f(__VA_ARGS__); \
    if (!_r) perror(#f), throw std::runtime_error(strerror(errno)); \
    _r; \
})

#endif//TRY_H
