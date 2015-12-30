#ifndef INTRUSIVE_TREE_H
#define INTRUSIVE_TREE_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include "compare.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
class intrusive_tree_link : private intrusive_link<X> {
public:
  typedef intrusive_tree_link type;
  template <class T, typename intrusive_tree_link<T>::type T::*link,
            typename K, K T::*key, compare_t (*C)(K const &, K const &)>
    friend class intrusive_tree;

  bool bound() const {
    assert(p.p || (!l.p && !r.p));
    return p.p; // tagged must mean black, since root has no parent
  }

private:
  intrusive_link_tag<X> p;
  intrusive_link<X> l, r;
};

template <class T, typename intrusive_tree_link<T>::type T::*link,
          typename K, K T::*key, compare_t (*C)(K const &, K const &) = compare<K> >
class intrusive_tree : public do_not_copy {
public:
  intrusive_tree() : root_(NULL) { }
  ~intrusive_tree() { assert(empty()); }

  bool empty() const { return !root_; }

  intrusive_tree & graft(T* t) {
    assert(valid());
    assert(!is_bound(t));

    T ** i = &root_;
    while (*i) {
      (t->*link).p.p = *i;
      i = C(t->*key, (*i)->*key) < 0
        ? &((*i)->*link).l.p
        : &((*i)->*link).r.p
        ;
    }
    *i = t;

    assert(is_red(t));

    T* n = t;
    while (true) {
      assert(n);

      // case 1
      T* p = parent_(n);
      if (!p) {
        assert(is_red(n));
        if (is_red(n))
          set_black(n);
        break;
      }

      // case 2
      if (is_black(p))
        break;

      T* g = parent_(p);
      if (!g)
        break;

      // case 4
      T* u = peer(p);
      if (!u || is_black(u)) {
        if (is_right(n) && is_left(p)) {
          assert(is_black(g));
          assert(is_red(p));
          assert(is_red(n));

          rotate_left(p);

          assert(is_black(g));
          assert(is_red(p));
          assert(is_red(n));

          using std::swap;
          swap(n,p); // move to left
        } else if (is_left(n) && is_right(p)) {
          assert(is_black(g));
          assert(is_red(p));
          assert(is_red(n));

          rotate_right(p);

          assert(is_black(g));
          assert(is_red(p));
          assert(is_red(n));

          using std::swap;
          swap(n,p); // move to right
        }

        // case 5
        assert(!is_left(n) || is_left(p));
        assert(!is_right(n) || is_right(p));

        assert(is_black(g));
        assert(is_red(p));
        assert(is_red(n));

        if (is_left(n))
          rotate_right(g);
        else // if (is_right(n))
          rotate_left(g);

        set_red(g);
        set_black(p);

        assert(is_red(g));
        assert(is_black(p));
        assert(is_red(n));

        break;
      }

      // case 3
      assert(is_black(g));
      assert(is_red(p));
      assert(is_red(u));

      set_red(g);
      set_black(p);
      set_black(u);

      n = g; // to case 1
    }

    assert(is_member(t));
    assert(is_bound(t));
    assert(!empty());
    assert(valid());

    return *this;
  }

  T* prune(T* t) {
    assert(valid());
    assert(!empty());
    assert(is_bound(t));
    assert(is_member(t));

    // swap with previous or next leaf
    static int i = 0;
    if (left_(t) && right_(t))
      node_swap(t, (++i % 2) ? rightest_(left_(t)): leftest_(right_(t)));

    assert(!left_(t) || !right_(t));

    T* c = left_(t) ? take_left(t)
         : right_(t) ? take_right(t)
         : NULL;


    // trivial case
    if (is_red(t)) {
      if (c)
        replace(t, c);
      else
        unlink(t);

      assert(!is_bound(t));
      assert(valid());

      return t;
    }

    // swap with child
    if (c)
      replace(t, c);

    if (is_red(c)) {
      assert(c);
      set_black(c);

      assert(!is_bound(t));
      assert(valid());

      return t;
    }

    // t acts as null leaf

    // !case 1
    T* n = c ? c : t;
    while (!is_root(n)) {
      T* p = parent_(n);
      T* s = peer(n);

      // case 2
      if (is_red(s)) {
        assert(is_black(p));
        set_red(p);
        set_black(s);

        if (is_left(n))
          rotate_left(p);
        else // if (is_right(n))
          rotate_right(p);

        p = parent_(n);
        s = peer(n);
      }

      T* sl = s ? left_(s) : NULL;
      T* sr = s ? right_(s) : NULL;

      // case 4
      if (is_red(p) && is_black(s) && is_black(sl) && is_black(sr)) {
        assert(s);
        set_red(s);
        set_black(p);
        break;
      }

      // case 5
      if (is_red(p) || is_red(sl) || is_red(sr)) {
        assert(is_black(s));
        if (is_left(n) && is_black(sr)) {
          assert(is_red(sl));
          set_red(s);
          set_black(sl);
          rotate_right(s);
        } else if (is_right(n) && is_black(sl)) {
          assert(is_red(sr));
          set_red(s);
          set_black(sr);
          rotate_left(s);
        }

        // case 6
        p = parent_(n);
        s = peer(n);
        sl = left_(s);
        sr = right_(s);

        if (is_red(p)) {
          set_black(p);
          if (is_black(s))
            set_red(s);
        }

        if (is_left(n)) {
          if (is_red(sr))
            set_black(sr);
          rotate_left(p);
        } else /* if (is_right(n)) */ {
          if (is_red(sl))
            set_black(sl);
          rotate_right(p);
        }

        break;
      }

      // case 3
      assert(is_black(p));
      assert(is_black(s));
      assert(is_black(sl));
      assert(is_black(sr));

      assert(s);
      set_red(s);
      n = p; // move to parent
    }

    if (!c)
      unlink(t);

    assert(!is_bound(t));
    assert(valid());

    return t;
  }

  T* transplant(T* o, T* n) {
    assert(valid());
    assert(is_bound(o));
    assert(is_member(o));
    assert(!is_bound(n));
    assert(!is_member(n));

    assert(0 == C(o->*key, n->*key));

    assert(is_red(n));
    if (is_black(o)) {
      set_red(o);
      set_black(n);
    }

    if (left_(o))
      link_left(n, take_left(o));

    if (right_(o))
      link_right(n, take_right(o));

    assert(!left_(o));
    assert(!right_(o));

    replace(o, n);

    assert(!is_member(o));
    assert(!is_bound(o));
    assert(is_member(n));
    assert(is_bound(n));
    assert(valid());

    return o;
  }

  intrusive_tree & inosculate(intrusive_tree & that) {
    assert(this != &that);

    T* i = that.root_;
    while (i) {
      if (T* l = left_(i))
        i = l;
      else if (T* r = right_(i))
        i = r;
      else {
        T* p = parent_(i);
        that.unlink(i);
        this->graft(i);
        i = p;
      }
    }

    assert(that.empty());

    return *this;
  }

  void swap(intrusive_tree & that) {
    using std::swap;
    swap(this->root_, that.root_);
  }

  bool is_member(const T* n) const {
    assert(n);
    return !empty() && eldest_(n) == root_;
  }

  T* root() const {
    assert(!empty());
    return root_;
  }

  T* parent(const T* n) const {
    assert(is_member(n));
    return parent_(n);
  }

  T* left(const T* n) const {
    assert(is_member(n));
    return left_(n);
  }

  T* right(const T* n) const {
    assert(is_member(n));
    return right_(n);
  }

  T* eldest(const T* n) const {
    assert(is_member(n));
    return eldest_(n);
  }

  T* leftest(const T* n) const {
    assert(is_member(n));
    return leftest_(n);
  }

  T* rightest(const T* n) const {
    assert(is_member(n));
    return rightest_(n);
  }

  T* min() const { return leftest(root()); }
  T* max() const { return rightest(root()); }

  T* next(const T* n) const {
    assert(is_member(n));

    if (T* r = right_(n))
      return leftest_(r);

    do {
      T* p = parent_(n);
      if (is_left(n))
        return p;
      n = p;
    } while (!is_root(n));

    return NULL;
  }

  T* prev(const T* n) const {
    assert(is_member(n));

    if (T* r = left_(n))
      return rightest_(r);

    do {
      T* p = parent_(n);
      if (is_right(n))
        return p;
      n = p;
    } while (!is_root(n));

    return NULL;
  }

  T* find(const K & k) const {
    T* n = root();

    while (n) {
      switch(C(k, n->*key)) {
      case -1: n = left(n); break;
      case  0: return n;
      case  1: n = right(n); break;
      default: assert(!"unreachable");
      }
    }

    return NULL;
  }

private:
  T * root_;

  static bool is_red(const T* n) { return n && !(n->*link).p.tagged(); }
  static bool is_black(const T* n) { return !n || (n->*link).p.tagged(); }
  static bool is_bound(const T* n) { assert(n); return (n->*link).bound(); }

  static void toggle(T* n) { assert(n); (n->*link).p.toggle(); }

  static void set_red(T* n) { assert(is_black(n)); toggle(n); }
  static void set_black(T* n) { assert(is_red(n)); toggle(n); }

  static T* parent_(const T* n) { assert(n); return (n->*link).p.tagless(); }
  static T* left_(const T* n) { assert(n); return (n->*link).l.p; }
  static T* right_(const T* n) { assert(n); return (n->*link).r.p; }

  static T* eldest_(const T* n) {
    assert(n);
    while (const T* c = parent_(n))
      n = c;
    return const_cast<T*>(n);
  }

  static T* leftest_(const T* n) {
    assert(n);
    while (const T* c = left_(n))
      n = c;
    return const_cast<T*>(n);
  }

  static T* rightest_(const T* n) {
    assert(n);
    while (const T* c = right_(n))
      n = c;
    return const_cast<T*>(n);
  }

  T* peer(const T* n) const {
    assert(!is_root(n));
    return is_left(n) ? right_(parent_(n)) : left_(parent_(n));
  }

  bool is_root(const T* n) const {
    assert(parent_(n) || n == root_);
    return !empty() && n == root_;
  }

  bool is_left(const T* n) const {
    assert(!is_root(n));
    return n == left_(parent_(n));
  }

  bool is_right(const T* n) const {
    assert(!is_root(n));
    return n == right_(parent_(n));
  }

  void unlink(T* n) {
    assert(n);
    assert(is_member(n));
    assert(!left_(n));
    assert(!right_(n));

    if (is_root(n))
      link_root(NULL);
    else if (is_left(n))
      link_left(parent_(n), NULL);
    else // if (is_right(n))
      link_right(parent_(n), NULL);

    (n->*link).p.p = NULL;
  }

  void link_root(T* n) {
    root_ = n;
    if (n)
      link_parent(NULL, n);
  }

  void link_left(T* p, T* c) {
    assert(p);
    if (c)
      link_parent(p, c);
    (p->*link).l.p = c;
  }

  void link_right(T* p, T* c) {
    assert(p);
    if (c)
      link_parent(p, c);
    (p->*link).r.p = c;
  }

  void link_parent(T* p, T* c) {
    assert(c);
    bool black = is_black(c);
    (c->*link).p.p = p;
    if (black)
      set_black(c);
  }

  void rotate_left(T* p) {
    T* g = parent_(p);
    T* n = right_(p);
    T* c = left_(n);
    if (is_root(p))
      link_root(n);
    else if (is_left(p))
      link_left(g, n);
    else // if (is_right(p))
      link_right(g, n);

    link_left(n, p);
    link_right(p, c);
  }

  void rotate_right(T* p) {
    T* g = parent_(p);
    T* n = left_(p);
    T* c = right_(n);
    if (is_root(p))
      link_root(n);
    else if (is_left(p))
      link_left(g, n);
    else // if (is_right(p))
      link_right(g, n);

    link_right(n, p);
    link_left(p, c);
  }

  void root_swap(T* n) {
    assert(n);

    if (is_left(n))
      link_left(parent_(n), root_);
    else // if (is_right(n))
      link_right(parent_(n), root_);

    link_root(n);
  }

  void left_swap(T* foo, T* bar) {
    T* fc = left_(foo);
    T* bc = left_(bar);

    link_left(foo, bc);
    link_left(bar, fc);
  }

  void right_swap(T* foo, T* bar) {
    T* fc = right_(foo);
    T* bc = right_(bar);

    link_right(foo, bc);
    link_right(bar, fc);
  }

  void parent_swap(T* foo, T* bar) {
    assert(!is_root(foo));
    assert(!is_root(bar));

    T* fp = parent_(foo);
    T* bp = parent_(bar);

    bool fl = is_left(foo);
    bool bl = is_left(bar);

    if (fl)
      link_left(fp, bar);
    else // if (fr)
      link_right(fp, bar);

    if (bl)
      link_left(bp, foo);
    else // if (br)
      link_right(bp, foo);
  }

  void node_swap(T* foo, T* bar) {
    if (is_root(foo))
      root_swap(bar);
    else if (is_root(bar))
      root_swap(foo);
    else
      parent_swap(foo, bar);

    if (is_black(foo) != is_black(bar)) {
      toggle(bar);
      toggle(foo);
    }

    left_swap(foo, bar);
    right_swap(foo, bar);
  }

  T* take_left(T* t) {
    T* c = left_(t);
    assert(c);
    link_left(t, NULL);
    link_parent(NULL, c);
    return c;
  }

  T* take_right(T* t) {
    T* c = right_(t);
    assert(c);
    link_right(t, NULL);
    link_parent(NULL, c);
    return c;
  }

  void replace(T* o, T* n) {
    if (is_root(o)) {
      unlink(o);
      link_root(n);
    } else if (is_left(o)) {
      T* p = parent_(o);
      unlink(o);
      link_left(p, n);
    } else /* if (is_right(t)) */ {
      T* p = parent_(o);
      unlink(o);
      link_right(p, n);
    }
  }

  unsigned depth(const T* t) const {
    if (!t)
      return 1;
    assert(depth(left_(t)) == depth(right_(t)));
    return (is_black(t) ? 1 : 0) + depth(left_(t));
  }

  bool valid(const T* t) const {
    if (!t)
      return true;

    T* l = left_(t);
    T* r = right_(t);

    if (is_red(t)) {
      if (l && !is_black(l))
        return false;
      if (r && !is_black(r))
        return false;
    }

    if (l && C(t->*key, l->*key) < 0)
      return false;
    if (r && C(r->*key, t->*key) < 0)
      return false;

    return valid(l) && valid(r);
  }

  bool valid() const {
    if (empty())
      return true;
    if (!is_black(root_))
      return false;
    if (depth(left_(root_)) != depth(right_(root_)))
      return false;
    return valid(root_);
  }

};

#endif//INTRUSIVE_TREE
