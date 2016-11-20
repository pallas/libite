#ifndef INTRUSIVE_HEAP_H
#define INTRUSIVE_HEAP_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include "compare.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
class intrusive_heap_link {
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
  intrusive_link_tag<X> s;
  intrusive_link<X> c;
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

    link_parent(NULL, t);
    link_root(empty() ? t : meld(root_, t));

    assert(is_bound(t));
    assert(!empty());
    assert(valid());

    return *this;
  }

  intrusive_heap & meld(intrusive_heap & that) {
    assert(this != &that);

    if (T* r = that.take_root())
      link_root(empty() ? r : meld(root_, r));

    assert(that.empty());

    return *this;
  }

  T* exhume() {
    assert(valid());
    assert(!empty());

    T* m = take_root();

    if (child(m)) {
      link_root(pass(m));
      link_parent(NULL, root_);
    }

    unlink(m);

    assert(!is_bound(m));
    assert(valid());

    return m;
  }

  T* sift(T* t) {
    assert(valid());
    assert(is_bound(t));

    if (t == root_)
      return exhume();

    T* p = parent(t);
    if (t == child(p)) {
      take_child(p);
    } else {
      T* c = child(p);
      while (t != sibling(c))
        c = sibling(c);

      take_sibling(c);

      if (!sibling(c))
        link_parent(p, c);
    }

    if (child(t))
      make_child(p, pass(t));

    unlink(t);

    assert(!is_bound(t));
    assert(valid());

    return t;
  }

  T* root() const {
    assert(!empty());
    return root_;
  }

  T* next(const T* n) const {
    assert(is_bound(n));

    if (T* c = child(n))
      return c;

    do {
      if (T* s = sibling(n))
        return s;
    } while (n = parent(n));

    return NULL;
  }

private:
  T * root_;

  static bool is_bound(const T* n) { assert(n); return (n->*link).bound(); }
  static bool is_baby(const T* n) { assert(n); return (n->*link).s.tagged(); }

  static T* sibling(const T* n) { assert(n); return (n->*link).s.qualified(!is_baby(n)); }
  static T* child(const T* n) { assert(n); return (n->*link).c.p; }

  static T* parent(const T* n) {
    assert(n);
    while (const T* s = sibling(n))
      n = s;
    return (n->*link).s.tagless();
  }

  T* meld(T* foo, T* bar) {
    assert(foo);
    assert(bar);

    using std::swap;
    if (C(bar->*key, foo->*key) < 0)
      swap(foo, bar);

    assert(!sibling(bar));
    make_child(foo, bar);

    return foo;
  }

  T* pass(T* t) {
    assert(child(t));
    assert(!sibling(t));

    // pairing pass
    T* r = NULL;
    while (child(t)) {
      T* c = take_child(t);
      if (child(t))
        c = meld(c, take_child(t));
      link_sibling(c, r);
      r = c;
    }
    assert(r);

    // melding pass
    T* ss = take_siblings(r);
    while (T* s = ss) {
      ss = take_siblings(ss);
      r = meld(r, s);
    }

    return r;
  }

  void link_root(T* n) {
    root_ = n;
  }

  void link_parent(T* p, T* c) {
    assert(c);
    (c->*link).s.tag(p);
  }

  void link_child(T* p, T* c) {
    assert(p);
    (p->*link).c.p = c;
  }

  void link_sibling(T* n, T* s) {
    assert(n);
    (n->*link).s.p = s;
  }

  void make_child(T* p, T* c) {
    assert(p);
    assert(c);
    if (T* cs = child(p))
      link_sibling(c, cs);
    else
      link_parent(p, c);
    link_child(p, c);
    assert(parent(child(p)) == p);
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

    if (!is_bound(t))
      return false;

    for (T* c = child(t) ; c ; c = sibling(c))
      if (C(c->*key, t->*key) < 0 || parent(c) != t || !valid(c))
        return false;

    return valid(sibling(t));
  }

  bool valid() const {
    if (empty())
      return true;
    assert(!parent(root_));
    assert(!sibling(root_));
    return valid(root_);
  }

};

#endif//INTRUSIVE_HEAP_H
