#ifndef INTRUSIVE_SET_H
#define INTRUSIVE_SET_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
struct intrusive_set_link {
  typedef intrusive_set_link type;
  template <class T, typename intrusive_set_link<T>::type T::*link>
    friend class intrusive_set;

  bool bound() const {
    assert(p.p || !n.p);
    return p.p;
  }

private:
  intrusive_link_tag<X> p;
  intrusive_link<X> n;
};

template <class T, typename intrusive_set_link<T>::type T::*link>
class intrusive_set : public do_not_copy {
public:
  intrusive_set() : head(NULL), tail(&head), rank(0) { }
  ~intrusive_set() { assert(empty()); }

  bool empty() const { return !head; }

  intrusive_set & join(T* t) {
    assert(!typed(t));
    assert(!empty() || &head == tail);

    if (!empty()) {
      (t->*link).p.p = head;
    } else {
      if (!rank)
        ++rank;
      (t->*link).p.tag(reinterpret_cast<T*>(this));
    }

    *tail = t;
    tail = &(t->*link).n.p;
    *tail = t;

    assert(typed(t));
    assert(!empty());
    return *this;
  }

  static bool typed(const T* t) { return (t->*link).bound(); }

  static intrusive_set* archetype(const T* t) {
    assert(typed(t));

    while (!(t->*link).p.tagged()) {
      const T* p = (t->*link).p.p;
      if (!(p->*link).p.tagged()) // compress path
        (const_cast<T*>(t)->*link).p.p = (p->*link).p.p;
      t = p;
    }

    return reinterpret_cast<intrusive_set*>((t->*link).p.tagless());
  }

  bool contains(const T* t) const {
    assert(typed(t));
    return this == archetype(t);
  }

  intrusive_set* conjoin(intrusive_set* that) {
    assert(this != that);

    if (that->empty())
      return that;

    // union by rank
    if (empty() || rank < that->rank) {
      std::swap(head, that->head);
      std::swap(tail, that->tail);
      std::swap(rank, that->rank);

      (head->*link).p.tag(reinterpret_cast<T*>(this));
    } else if (rank == that->rank)
      ++rank;

    if (!that->empty()) {
      assert((head->*link).p.tagged());
      (that->head->*link).p.p = head;

      *tail = that->head;
      tail = that->tail;
    }

    that->head = NULL;
    that->tail = &that->head;
    that->rank = 0;

    assert(that->empty());
    return that;
  }

  typedef void (T::*dissolver_t)();

  intrusive_set & dissolve(dissolver_t d = NULL) {
    while (T* t = head) {
      assert(typed(t));
      head = t != *tail ? (t->*link).n.p : NULL;
      (t->*link).p.p = NULL;
      (t->*link).n.p = NULL;
      assert(!typed(t));
      if (d)
        (t->*d)();
    }

    tail = &head;
    rank = 0;

    assert(empty());
    return *this;
  }

  T* iterator() const { return head; }

  T* next(const T* t) const {
    assert(contains(t));
    return t != *tail ? (t->*link).n.p : NULL;
  }

private:
  T * head;
  T ** tail;

  unsigned rank;
};

#endif//INTRUSIVE_SET_H
