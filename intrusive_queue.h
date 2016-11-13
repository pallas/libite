#ifndef INTRUSIVE_QUEUE_H
#define INTRUSIVE_QUEUE_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include "compare.h"

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

  T* last() const {
    assert(!empty());
    return *tail;
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

  template <typename K, K T::*key, compare_t (*C)(K const &, K const &)>
  intrusive_queue & sort() {
    if (this->empty() || this->head == *this->tail)
      return *this;

    for (unsigned size = 1 ; ; size *= 2) {
      intrusive_queue that;
      while (!this->empty()) {
        intrusive_queue foo, bar;

        assert(!this->empty());
        foo.chain(*this, size);
        assert(!foo.empty());

        if (this->empty()) {
          if (!that.empty()) {
            that.chain(foo);
            break;
          } else {
            this->chain(foo);
            return *this;
          }
        }

        assert(!this->empty());
        bar.chain(*this, size);
        assert(!bar.empty());

        while (!foo.empty() && !bar.empty())
          if (C(foo.peek()->*key, bar.peek()->*key) <= 0)
            that.enqueue(foo.dequeue());
          else
            that.enqueue(bar.dequeue());

        assert(!that.empty());

        if (!foo.empty()) {
          assert(C((*that.tail)->*key, foo.peek()->*key) <= 0);
          that.chain(foo);
        } else if (!bar.empty()) {
          assert(C((*that.tail)->*key, bar.peek()->*key) <= 0);
          that.chain(bar);
        }
      }

      assert(!that.empty());
      this->chain(that);
    }

    assert(!"unreachable");
  }

  template <typename K, K T::*key>
  intrusive_queue & sort() {
    return sort<K, key, compare<K> >();
  }

  intrusive_queue & chain(intrusive_queue & that, unsigned n) {
    assert(this != &that);
    assert(!that.empty());

    T* self = *tail;
    *tail = that.head;

    for (unsigned i = 0 ; i < n && tail != that.tail ; ++i) {
      self = *tail;
      tail = &(self->*link).p;
    }

    if (tail == that.tail) {
      that.head = NULL;
      that.tail = &that.head;
    } else {
      that.head = *tail;
      *tail = self;
    }

    return *this;
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

  T* iterator() const { return head; }

  T* next(const T* t) const {
    assert((t->*link).bound());
    return t != *tail ? (t->*link).p : NULL;
  }

private:
  T * head;
  T ** tail;
};

#endif//INTRUSIVE_QUEUE
