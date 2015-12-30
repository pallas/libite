#ifndef INTRUSIVE_LINK_H
#define INTRUSIVE_LINK_H

#include <cassert>
#include <cstddef>
#include <stdint.h>

template <class T>
struct intrusive_link {
  intrusive_link() : p(NULL) { }
  ~intrusive_link() { assert(!p); }

  T* p;
};

template <class T, intptr_t TAG = 0x1>
struct intrusive_link_tag : public intrusive_link<T> {
  typedef intrusive_link<T> link;

  void tag(T* t) { link::p = (T*)(intptr_t(t) | TAG); }
  void toggle() { link::p = (T*)(intptr_t(link::p) ^ TAG); }

  bool tagged() const { return intptr_t(link::p) & TAG; }
  T* tagless() const { return (T*)(intptr_t(link::p) & ~TAG); }
};

#endif//INTRUSIVE_LINK_H
