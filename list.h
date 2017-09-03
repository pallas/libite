#ifndef LITE__LIST_H
#define LITE__LIST_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

namespace lite {

template <class X>
struct list_link {
  typedef list_link type;
  template <class T, typename list_link<T>::type T::*L>
    friend class list;

  bool bound() const { return p.p; }

private:
  link<X> n, p;
};

template <class T, typename list_link<T>::type T::*L>
class list : public lace::do_not_copy {
public:
  list() : head(NULL) { }
  ~list() { assert(empty()); }

  bool empty() const { return !head; }

  list & enlist(T* t, T* n = NULL) {
    assert(!(t->*L).bound());
    assert(!n || (n->*L).bound());

    (t->*L).n.p = (t->*L).p.p = t;

    if (empty()) {
      assert(!n);
      head = t;
    } else {
      if (n == head)
        head = t;
      else if (!n)
        n = head;

      T* p = (n->*L).p.p;
      std::swap((t->*L).n.p, (p->*L).n.p);
      std::swap((t->*L).p.p, (n->*L).p.p);
    }

    assert((t->*L).bound());
    assert(!empty());
    return *this;
  }

  T* delist(T* t) {
    assert((t->*L).bound());

    T* n = (t->*L).n.p;
    T* p = (t->*L).p.p;

    if (t == n) {
      assert(head == t);
      assert(p == n);
      head = NULL;
    } else {
      if (head == t)
        head = n;

      std::swap((t->*L).n.p, (p->*L).n.p);
      std::swap((t->*L).p.p, (n->*L).p.p);
    }

    (t->*L).n.p = (t->*L).p.p = NULL;

    assert(!(t->*L).bound());
    return t;
  }

  T* first() const { return head; }
  T* last() const { return head ? (head->*L).p.p : NULL; }

  T* next(const T* t) const {
    assert((t->*L).bound());
    return (t->*L).n.guarded(head);
  }

  T* previous(const T* t) const {
    assert((t->*L).bound());
    return (t->*L).p.qualified(t != head);
  }

  T* prograde() {
    assert(!empty());
    head = (head->*L).n.p;
    return head;
  }

  T* retrograde() {
    assert(!empty());
    head = (head->*L).p.p;
    return head;
  }

private:
  T * head;
};

} // namespace lite

#endif//LITE__LIST
