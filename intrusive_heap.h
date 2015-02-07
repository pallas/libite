#ifndef INTRUSIVE_HEAP_H
#define INTRUSIVE_HEAP_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include "compare.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
class intrusive_heap_link : private intrusive_link<X> {
public:
  typedef intrusive_heap_link type;
  template <class T, typename intrusive_heap_link<T>::type T::*link,
            typename K, K T::*key, compare_t (*C)(K const &, K const &)>
    friend class intrusive_heap;

  bool bound() const {
    assert(s.p || !c.p);
    return s.p;
  }

private:
  intrusive_link<X> s, c;
};

template <class T, typename intrusive_heap_link<T>::type T::*link,
          typename K, K T::*key, compare_t (*C)(K const &, K const &) = compare<K> >
class intrusive_heap : public do_not_copy {
public:

  intrusive_heap() : root_(NULL) { }
  ~intrusive_heap() { assert(empty()); }

  bool empty() const { return !root_; }

  intrusive_heap & inhume(T* t) {
    assert(valid());
    assert(!is_bound(t));

    link_sibling(t, NULL);
    link_root(empty() ? t : meld(root_, t));

    assert(is_bound(t));
    assert(!empty());
    assert(valid());

    return *this;
  }

  T* exhume() {
    assert(valid());
    assert(!empty());

    T* m = take_root();

    // pairing pass
    while (child(m)) {
      T* c = take_child(m);
      if (child(m))
        c = meld(c, take_child(m));
      link_sibling(c, root_);
      link_root(c);
    }

    // melding pass
    if (root_) {
      while (sibling(root_)) {
        T* s = take_siblings(root_);
        T* r = take_siblings(s);
        link_root(meld(root_, s));
        link_sibling(root_, r);
      }
    }

    unlink(m);

    assert(!is_bound(m));
    assert(valid());

    return m;
  }

  T* root() const {
    assert(!empty());
    return root_;
  }

private:
  T * root_;

  static bool is_bound(const T* n) { assert(n); return (n->*link).bound(); }
  static bool is_baby(const T* n) { assert(n); return n == (n->*link).s.p; }

  static T* sibling(const T* n) { assert(n); return !is_baby(n) ? (n->*link).s.p : NULL; }
  static T* child(const T* n) { assert(n); return (n->*link).c.p; }

  T* meld(T* foo, T* bar) {
    assert(foo);
    assert(bar);

    using std::swap;
    if (C(bar->*key, foo->*key) < 0)
      swap(foo, bar);

    assert(!sibling(bar));
    link_sibling(bar, child(foo));
    link_child(foo, bar);

    return foo;
  }

  void link_root(T* n) {
    root_ = n;
  }

  void link_child(T* p, T* c) {
    assert(p);
    (p->*link).c.p = c;
  }

  void link_sibling(T* n, T* s) {
    assert(n);
    (n->*link).s.p = s ? s : n;
  }

  void unlink(T* n) {
    assert(n);
    assert(!sibling(n));
    assert(!child(n));

    (n->*link).s.p = NULL;
  }

  T* take_root() {
    T* r = NULL;
    using std::swap;
    swap(root_, r);
    return r;
  }

  T* take_child(T* t) {
    T* c = child(t);
    assert(c);
    link_child(t, sibling(c));
    link_sibling(c, NULL);
    return c;
  }

  T* take_children(T* t) {
    T* c = child(t);
    link_child(t, NULL);
    return c;
  }

  T* take_sibling(T* t) {
    T* s = sibling(t);
    assert(s);
    link_sibling(t, sibling(s));
    link_sibling(s, NULL);
    return s;
  }

  T* take_siblings(T* t) {
    T* s = sibling(t);
    link_sibling(t, NULL);
    return s;
  }

  bool valid(const T* t) const {
    if (!t)
      return true;

    for (T* c = child(t) ; c ; c = sibling(c))
      if (C(c->*key, t->*key) < 0 || !valid(c))
        return false;

    return valid(sibling(t));
  }

  bool valid() const {
    if (empty())
      return true;
    assert(!sibling(root_));
    return valid(root_);
  }

};

#endif//INTRUSIVE_HEAP_H
