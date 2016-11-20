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

    head = (head->*link).qualified(*tail != head);

    if (empty())
      tail = &head;

    (t->*link).p = NULL;

    assert(!(t->*link).bound());
    return t;
  }

  template <typename K, K T::*key,
            compare_t (*C)(K const &, K const &) = compare<K> >
  struct sorter {

    static
    compare_t compare(const T* foo, const T* bar) {
      return C(foo->*key, bar->*key);
    }

    static
    bool sorted(const intrusive_queue & q) {
      if (!q.empty())
        for (T* i = q.peek() ; i ; i = q.next(i))
          if (T* n = q.next(i))
            if (compare(i, n) > 0)
              return false;

      return true;
    }

    static
    intrusive_queue & merge(intrusive_queue & q,
                           intrusive_queue & foo,
                           intrusive_queue & bar)
    {
      assert(sorted(foo));
      assert(sorted(bar));

      while (!foo.empty() && !bar.empty())
        if (compare(foo.peek(), bar.peek()) <= 0)
          q.enqueue(foo.dequeue());
        else
          q.enqueue(bar.dequeue());

      if (!foo.empty())
        q.chain(foo);

      if (!bar.empty())
        q.chain(bar);

      assert(foo.empty());
      assert(bar.empty());
      return q;
    }

    static
    intrusive_queue & sort(intrusive_queue & q) {
      if (q.empty() || q.peek() == q.last())
        return q;

      for (unsigned size = 1 ; ; size *= 2) {
        intrusive_queue that;
        while (!q.empty()) {
          intrusive_queue foo, bar;

          assert(!q.empty());
          foo.chain(q, size);
          assert(!foo.empty());
          assert(sorted(foo));

          if (q.empty()) {
            if (!that.empty()) {
              that.chain(foo);
              break;
            } else {
              q.chain(foo);
              assert(sorted(q));
              return q;
            }
          }

          assert(!q.empty());
          bar.chain(q, size);
          assert(!bar.empty());
          assert(sorted(bar));

          merge(that, foo, bar);
          assert(!that.empty());
        }

        assert(!that.empty());
        q.chain(that);
      }

      assert(!"unreachable");
    }

  }; // sorter

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
    return (t->*link).qualified(t != *tail);
  }

private:
  T * head;
  T ** tail;
};

#endif//INTRUSIVE_QUEUE
