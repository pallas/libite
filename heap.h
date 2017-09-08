#ifndef LITE__HEAP_H
#define LITE__HEAP_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <lace/compare.h>

#include <cassert>
#include <cstddef>
#include <algorithm>

namespace lite {

template <class X>
class heap_link {
public:
  typedef heap_link type;
  template <class T, typename heap_link<T>::type T::*L,
            typename K, K T::*key, lace::compare_t (*C)(K const &, K const &)>
    friend class heap;

  bool bound() const {
    assert(s.p || !c.p);
    return s.p;
  }

private:
  link_tag<X> s;
  link<X> c;
};

template <class T, typename heap_link<T>::type T::*L,
          typename K, K T::*key, lace::compare_t (*C)(K const &, K const &) = lace::compare<K> >
class heap : public lace::do_not_copy {
public:

  heap() : root_(NULL) { }
  ~heap() { assert(empty()); }

  bool empty() const { return !root_; }

  heap & inhume(T* t) {
    assert(valid());
    assert(!is_bound(t));

    link_parent(NULL, t);
    link_root(empty() ? t : meld(root_, t));

    assert(is_bound(t));
    assert(!empty());
    assert(valid());

    return *this;
  }

  heap & meld(heap & that) {
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

    orphan(t);

    assert(!is_bound(t));
    assert(valid());

    return t;
  }

  bool rehume(T* t) {
    assert(is_bound(t));

    if (t == root_)
      return false;

    T* p = parent(t);
    if (C(p->*key, t->*key) <= 0)
      return false;

    orphan(t, p);

    link_parent(NULL, t);
    link_root(meld(root_, t));

    assert(is_bound(t));
    assert(valid());

    return true;
  }

  heap & bury(T* t) {
    assert(is_bound(t));

    if (!child(t))
      return *this;

    T* cs = pass(t);

    if (C(t->*key, cs->*key) <= 0) {
       make_child(t, cs);
    } else if (t == root_) {
      link_parent(NULL, cs);
      link_root(meld(root_, cs));
    } else {
      make_sibling(t, cs);
    }

    assert(is_bound(t));
    assert(valid());

    return *this;
  }

  heap & churn(T* t) {
    if (!is_bound(t))
      return inhume(t);

    rehume(t);
    return *this;
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

  static bool is_bound(const T* n) { assert(n); return (n->*L).bound(); }
  static bool is_baby(const T* n) { assert(n); return (n->*L).s.tagged(); }

  static T* sibling(const T* n) { assert(n); return (n->*L).s.qualified(!is_baby(n)); }
  static T* child(const T* n) { assert(n); return (n->*L).c.p; }

  static T* parent(const T* n) {
    assert(n);
    while (const T* s = sibling(n))
      n = s;
    return (n->*L).s.tagless();
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
    (c->*L).s.tag(p);
  }

  void link_child(T* p, T* c) {
    assert(p);
    (p->*L).c.p = c;
  }

  void link_sibling(T* n, T* s) {
    assert(n);
    (n->*L).s.p = s;
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

  void make_sibling(T* n, T* s) {
    assert(n);
    assert(s);
    if (sibling(n))
      link_sibling(s, take_siblings(n));
    else
      link_parent(parent(n), s);
    link_sibling(n, s);
  }

  void unlink(T* n) {
    assert(n);
    assert(!sibling(n));
    assert(!child(n));

    (n->*L).s.p = NULL;
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

  void orphan(T* t) {
    assert(t != root_);
    orphan(t, parent(t));
  }

  void orphan(T* c, T* p) {
    assert(c != root_);
    assert(p == parent(c));
    if (c == child(p)) {
      take_child(p);
    } else {
      T* s = child(p);
      while (c != sibling(s))
        s = sibling(s);

      take_sibling(s);

      if (!sibling(s))
        link_parent(p, s);
    }

    if (child(c))
      make_child(p, pass(c));

    unlink(c);
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

} // namespace lite

#endif//LITE__HEAP_H
