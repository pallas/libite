#ifndef COMPARE_H
#define COMPARE_H

#include <cwchar>
#include <cstring>
#include <stdint.h>

// return -1 if foo < bar
// return  0 if foo = bar
// return +1 if foo > bar

typedef int_fast8_t compare_t;

template <typename T>
compare_t compare(T const &foo, T const &bar) {
  return (bar<foo) - (foo<bar);
}

template <>
compare_t compare(const char * const &foo, const char * const &bar) {
  return compare<int>(strcmp(foo, bar), 0);
}

template <>
compare_t compare(const wchar_t * const &foo, const wchar_t * const &bar) {
  return compare<int>(wcscmp(foo, bar), 0);
}

template <typename T>
compare_t reverse_compare(T const &foo, T const &bar) {
  return -compare(foo, bar);
}

#endif//COMPARE_H
