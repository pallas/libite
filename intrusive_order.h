#ifndef INTRUSIVE_ORDER_H
#define INTRUSIVE_ORDER_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
struct intrusive_order_link : private intrusive_link<X> {
  typedef intrusive_order_link type;
  template <class T, typename intrusive_order_link<T>::type T::*link,
            typename K, K T::*key>
    friend class intrusive_order;

  bool bound() const { return intrusive_link<X>::p; }
};

template <class T, typename intrusive_order_link<T>::type T::*link,
          typename K, K T::*key>
class intrusive_order : public do_not_copy {
public:
  intrusive_order() : head(NULL), tail(&head) { }
  ~intrusive_order() { assert(empty()); }

  bool empty() const { return !head; }

  intrusive_order & insert(T* t) {
    assert(!(t->*link).bound());
    assert(!empty() || &head == tail);

    if (empty() || !(t->*key < (*tail)->*key)) {
      *tail = t;
      tail = &(t->*link).p;
      *tail = t;
    } else {
      T ** c;
      for (c = &head ; c != tail ; c = &((*c)->*link).p)
        if (!((*c)->*key < t->*key))
          break;
      (t->*link).p = *c;
      *c = t;
    }

    assert((t->*link).bound());
    assert(!empty());
    return *this;
  }

  T & front() {
    assert(!empty());
    return *head;
  }

  const T & front() const {
    assert(!empty());
    return *head;
  }

  T & back() {
    assert (!empty());
    return **tail;
  }

  const T & back() const {
    assert (!empty());
    return **tail;
  }

  T* remove() {
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

  intrusive_order & merge(intrusive_order & that) {
    assert(this != &that);
    intrusive_order result;

    while (!this->empty() && !that.empty())
      result.insert( this->head->*key < that.head->*key
                   ? this->remove()
                   : that.remove() );

    if (!this->empty())
      result.chain(*this);

    if (!that.empty())
      result.chain(that);

    swap(result);
    return *this;
  }

  void swap(intrusive_order & that) {
    std::swap(this->head, that.head);
    std::swap(this->tail, that.tail);
  }

private:
  T * head;
  T ** tail;

  intrusive_order & chain(intrusive_order & that) {
    assert(this != &that);
    assert(!that.empty());

    *tail = that.head;
    tail = that.tail;
    assert(!empty());

    that.head = NULL;
    that.tail = &that.head;
    assert(that.empty());

    return *this;
  }
};

#endif//INTRUSIVE_ORDER
