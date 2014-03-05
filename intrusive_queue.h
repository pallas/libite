#ifndef INTRUSIVE_QUEUE_H
#define INTRUSIVE_QUEUE_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include <cassert>
#include <cstddef>

template <class X>
struct intrusive_queue_link : private intrusive_link<X> {
  typedef intrusive_queue_link type;
  template <class T, typename intrusive_queue_link<T>::type T::*link>
    friend class intrusive_queue;

  bool bound() const { return intrusive_link<X>::p; }
};

template <class T, typename intrusive_queue_link<T>::type T::*link>
class intrusive_queue : public do_not_copy {
public:
  intrusive_queue() : head(NULL), tail(&head) { }
  ~intrusive_queue() { assert(empty()); }

  bool empty() const { return !head; }

  intrusive_queue & enqueue(T* t) {
    assert(!(t->*link).bound());
    assert(!empty() || &head == tail);

    *tail = t;
    tail = &(t->*link).p;
    *tail = t;

    assert((t->*link).bound());
    assert(!empty());
    return *this;
  }

  T* peek() const {
    assert(!empty());
    return head;
  }

  T* dequeue() {
    assert(!empty());

    T* t = head;
    assert((t->*link).bound());

    head = *tail != head
         ? (head->*link).p
         : NULL;

    if (empty())
      tail = &head;

    (t->*link).p = NULL;

    assert(!(t->*link).bound());
    return t;
  }

  intrusive_queue & chain(intrusive_queue & that) {
    assert(this != &that);
    assert(!that.empty());
    *tail = that.head;
    tail = that.tail;

    that.head = NULL;
    that.tail = &that.head;

    assert(that.empty());
    return *this;
  }

private:
  T * head;
  T ** tail;
};

#endif//INTRUSIVE_QUEUE
