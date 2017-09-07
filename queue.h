#ifndef LITE__QUEUE_H
#define LITE__QUEUE_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <lace/compare.h>

#include <cassert>
#include <cstddef>

namespace lite {

template <class X>
struct queue_link : private link<X> {
  typedef queue_link type;
  template <class T, typename queue_link<T>::type T::*L>
    friend class queue;

  bool bound() const { return link<X>::p; }
};

template <class T, typename queue_link<T>::type T::*L>
class queue : public lace::do_not_copy {
public:
  queue() : head(NULL), tail(&head) { }
  ~queue() { assert(empty()); }

  bool empty() const { return !head; }

  queue & enqueue(T* t) {
    assert(!(t->*L).bound());
    assert(!empty() || &head == tail);

    *tail = t;
    tail = &(t->*L).p;
    *tail = t;

    assert((t->*L).bound());
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
    assert((t->*L).bound());

    head = (head->*L).qualified(*tail != head);

    if (empty())
      tail = &head;

    (t->*L).p = NULL;

    assert(!(t->*L).bound());
    return t;
  }

  template <typename K, K T::*key,
            lace::compare_t (*C)(K const &, K const &) = lace::compare<K> >
  struct sorter {

    static
    lace::compare_t compare(const T* foo, const T* bar) {
      return C(foo->*key, bar->*key);
    }

    static
    bool sorted(const queue & q) {
      if (!q.empty())
        for (T* i = q.peek() ; i ; i = q.next(i))
          if (T* n = q.next(i))
            if (compare(i, n) > 0)
              return false;

      return true;
    }

    static
    queue & merge(queue & q,
                           queue & foo,
                           queue & bar)
    {
      assert(sorted(foo));
      assert(sorted(bar));

      while (!foo.empty() && !bar.empty())
        if (compare(foo.peek(), bar.peek()) <= 0)
          q.chain(foo, 1);
        else
          q.chain(bar, 1);

      if (!foo.empty())
        q.chain(foo);

      if (!bar.empty())
        q.chain(bar);

      assert(foo.empty());
      assert(bar.empty());
      return q;
    }

    static
    queue & sort(queue & q) {
      if (q.empty() || q.peek() == q.last())
        return q;

      for (unsigned size = 1 ; ; size *= 2) {
        queue that;
        while (!q.empty()) {
          queue foo, bar;

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

  template <typename K, K T::*key>
  struct reverse_sorter : public sorter<K, key, lace::reverse_compare<K> > { };

  queue & reverse() {
    if (empty())
      return *this;

    T** nt = &(head->*L).p;

    T* p = head;
    while (head != *tail) {
      T* n = (head->*L).p;
      (head->*L).p = p;
      p = head;
      head = n;
    }
    (head->*L).p = p;
    tail = nt;

    return *this;
  }

  queue & chain(queue & that, unsigned n) {
    assert(this != &that);
    assert(!that.empty());

    T* self = *tail;
    *tail = that.head;

    for (unsigned i = 0 ; i < n && tail != that.tail ; ++i) {
      self = *tail;
      tail = &(self->*L).p;
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

  queue & chain(queue & that) {
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
    assert((t->*L).bound());
    return (t->*L).qualified(t != *tail);
  }

private:
  T * head;
  T ** tail;
};

} // namespace lite

#endif//LITE__QUEUE
