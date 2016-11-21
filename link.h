#ifndef LITE__LINK_H
#define LITE__LINK_H

#include <cassert>
#include <cstddef>
#include <stdint.h>

namespace lite {

template <class T>
struct link {
  link() : p(NULL) { }
  ~link() { assert(!p); }

  T* p;

  T* qualified(bool predicate) const { return (T*)(intptr_t(p) & -intptr_t(predicate)); }
  T* guarded(const T* sentinel) const { return qualified(p != sentinel); }
};

template <class T, intptr_t TAG = 0x1>
struct link_tag : public link<T> {
  typedef link<T> link_t;

  void tag(T* t) { link_t::p = (T*)(intptr_t(t) | TAG); }
  void toggle() { link_t::p = (T*)(intptr_t(link_t::p) ^ TAG); }

  bool tagged() const { return intptr_t(link_t::p) & TAG; }
  T* tagless() const { return (T*)(intptr_t(link_t::p) & ~TAG); }
};

} // namespace lite

#endif//LITE__LINK_H
