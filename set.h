#ifndef LITE__SET_H
#define LITE__SET_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

namespace lite {

template <class X>
struct set_link {
  typedef set_link type;
  template <class T, typename set_link<T>::type T::*L>
    friend class set;

  bool bound() const {
    assert(p.p || !n.p);
    return p.p;
  }

private:
  link_tag<X> p;
  link<X> n;
};

template <class T, typename set_link<T>::type T::*L>
class set : public lace::do_not_copy {
public:
  set() : head(NULL), tail(&head), rank(0) { }
  ~set() { assert(empty()); }

  bool empty() const { return !head; }

  set & join(T* t) {
    assert(!typed(t));
    assert(!empty() || &head == tail);

    if (!empty()) {
      (t->*L).p.p = head;
    } else {
      if (!rank)
        ++rank;
      (t->*L).p.tag(reinterpret_cast<T*>(this));
    }

    *tail = t;
    tail = &(t->*L).n.p;
    *tail = t;

    assert(typed(t));
    assert(!empty());
    return *this;
  }

  static bool typed(const T* t) { return (t->*L).bound(); }

  static set* archetype(const T* t) {
    assert(typed(t));

    while (!(t->*L).p.tagged()) {
      const T* p = (t->*L).p.p;
      if (!(p->*L).p.tagged()) // compress path
        (const_cast<T*>(t)->*L).p.p = (p->*L).p.p;
      t = p;
    }

    return reinterpret_cast<set*>((t->*L).p.tagless());
  }

  bool contains(const T* t) const {
    assert(typed(t));
    return this == archetype(t);
  }

  set* conjoin(set* that) {
    assert(this != that);

    if (that->empty())
      return that;

    // union by rank
    if (empty() || rank < that->rank) {
      std::swap(head, that->head);
      std::swap(tail, that->tail);
      std::swap(rank, that->rank);

      (head->*L).p.tag(reinterpret_cast<T*>(this));
    } else if (rank == that->rank)
      ++rank;

    if (!that->empty()) {
      assert((head->*L).p.tagged());
      (that->head->*L).p.p = head;

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

  set & dissolve(const dissolver_t d = NULL) {
    while (T* t = head) {
      assert(typed(t));
      head = (t->*L).n.qualified(t != *tail);
      (t->*L).p.p = NULL;
      (t->*L).n.p = NULL;
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
    return (t->*L).n.qualified(t != *tail);
  }

private:
  T * head;
  T ** tail;

  unsigned rank;
};

} // namespace lite

#endif//LITE__SET_H
