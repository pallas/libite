#ifndef INTRUSIVE_LINK_H
#define INTRUSIVE_LINK_H

#include <cassert>
#include <cstddef>

template <class T>
struct intrusive_link {
  intrusive_link() : p(NULL) { }
  ~intrusive_link() { assert(!p); }

  T* p;
};

#endif//INTRUSIVE_LINK_H
