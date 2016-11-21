#ifndef LITE__STACK_H
#define LITE__STACK_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <cassert>
#include <cstddef>

namespace lite {

template <class X>
struct stack_link : private link<X> {
  typedef stack_link type;
  template <class T, typename stack_link<T>::type T::*L>
    friend class stack;

  bool bound() const { return link<X>::p; }
};

template <class T, typename stack_link<T>::type T::*L>
class stack : public lace::do_not_copy {
public:
  stack() : head(const_cast<T*>(sentinel())) { }
  ~stack() { assert(empty()); }

  bool empty() const { return head == sentinel(); }

  stack & push(T* t) {
    assert(!(t->*L).bound());

    (t->*L).p = head;
    head = t;

    assert((t->*L).bound());
    assert(!empty());
    return *this;
  }

  T* peek() const {
    assert(!empty());
    return head;
  }

  T* pop() {
    assert(!empty());

    T* t = head;
    assert((t->*L).bound());

    head = (t->*L).p;
    (t->*L).p = NULL;

    assert(!(t->*L).bound());
    return t;
  }

  T* iterator() const { return !empty() ? head : NULL; }

  T* next(const T* t) const {
    assert((t->*L).bound());
    return (t->*L).guarded(sentinel());
  }

private:
  T * head;
  const T* sentinel() const { return reinterpret_cast<const T*>(&head); }
};

} // namespace lite

#endif//LITE__STACK_H
