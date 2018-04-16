#ifndef LITE__TABLE_H
#define LITE__TABLE_H

#include <lace/do_not_copy.h>
#include "link.h"

#include <lace/hash.h>
#include <lace/compare.h>
#include <lace/divider.h>

#include <cassert>
#include <cstddef>
#include <algorithm>

namespace lite {

template <class X>
struct table_link : private link<X> {
  typedef table_link type;
  template <class T, typename table_link<T>::type T::*L,
            typename K, K T::*key, lace::compare_t (*C)(K const &, K const &),
            lace::hash_t (*H)(K const &)>
    friend class table;

  bool bound() const { return link<X>::p; }
};

template <class X>
class table_bucket : private link<X> {
  template <class T, typename table_link<T>::type T::*L,
            typename K, K T::*key, lace::compare_t (*C)(K const &, K const &),
            lace::hash_t (*H)(K const &)>
    friend class table;
private:
  X* sentinel() { return reinterpret_cast<X*>(this); }
  const X* sentinel() const { return reinterpret_cast<const X*>(this); }
};

template <class T, typename table_link<T>::type T::*L,
          typename K, K T::*key,
          lace::compare_t (*C)(K const &, K const &) = lace::compare<K>,
          lace::hash_t (*H)(K const &) = lace::hash<K> >
class table : public lace::do_not_copy {
public:
  typedef table_bucket<T> bucket_t;

  table(bucket_t bs[] = NULL, const size_t n = 0)
    : buckets_(bs), n_buckets_(n), divider_(n)
  { assert(buckets_ || 0 == n_buckets_); set_buckets(); }

  ~table() { assert(empty()); reset_buckets(); }

  bool empty() const {
    assert(buckets_ || 0 == n_buckets_);

    for (size_t i = 0 ; i < n_buckets_ ; ++i)
      if (buckets_[i].p != buckets_[i].sentinel())
        return false;

    return true;
  }

  size_t buckets() const { return n_buckets_; }

  bool reseat(size_t n = 0) {
    if (n <= 0)
      delete [] dehash();
    else if (bucket_t *bs = new bucket_t[n])
      delete [] rehash(bs, n);
    else
      return false;

    return true;
  }

  bucket_t* rehash(bucket_t bs[], size_t n) {
    assert(buckets_ || 0 == n_buckets_);

    bucket_t ts;

    take_all(ts);
    reset_buckets();

    std::swap(buckets_, bs);
    std::swap(n_buckets_, n);
    divider_.invert(n_buckets_);

    set_buckets();
    give_all(ts);

    return bs;
  }

  bucket_t* dehash() {
    assert(empty());
    return rehash(NULL, 0);
  }

  table & set(T* t) {
    assert(buckets_);
    assert(!is_bound(t));

    insert_at(&buckets_[index(t)].p, t);

    assert(is_bound(t));
    assert(!empty());
  }

  T* bus(T* t) {
    assert(!empty());
    assert(is_bound(t));
    assert(is_member(t));

    bucket_t & b = buckets_[index(t)];
    assert(b.p);

    T ** c = &b.p;
    while (t != *c)
      c = &((*c)->*L).p;

    take_next(c);

    assert(!is_member(t));
    assert(!is_bound(t));

    return t;
  }

  T& operator[] (const K & k) const {
    T* t = get(k);
    assert(t);
    return *t;
  }

  T* get(const K & k) const {
    if (!n_buckets_)
      return NULL;

    bucket_t & b = buckets_[index(k)];

    for (T ** c = &b.p ; *c != b.sentinel() ; c = &((*c)->*L).p)
      if (0 == C(k, (*c)->*key))
          return c == &b.p ? *c : insert_at(&b.p, take_next(c));

    return NULL;
  }

  bool is_member(const T* t) const {
    if (!n_buckets_ || !is_bound(t))
      return false;

    bucket_t & b = buckets_[index(t)];

    for (T ** c = &b.p ; *c != b.sentinel() ; c = &((*c)->*L).p)
      if (t == *c)
        return true;

    return false;
  }

  T* iterator() const {
    for (size_t i = 0 ; i < n_buckets_ ; ++i)
      if (buckets_[i].p != buckets_[i].sentinel())
        return buckets_[i].p;

    return NULL;
  }

  T* next(const T* t) const {
    assert(is_member(t));

    T* n = (t->*L).p;

    if (bucket_t* b = is_bucket(n)) {
      size_t o = b - buckets_;
      for (size_t i = o + 1; i < n_buckets_ ; ++i)
        if (buckets_[i].p != buckets_[i].sentinel())
          return buckets_[i].p;

      return NULL;
    }

    return n;
  }

  typedef void (T::*wiper_t)();

  T* wipe(T* t, wiper_t w) {
    assert(w);
    T* n = next(t);
    (bus(t)->*w)();
    return n;
  }

  table & polish(wiper_t w) {
    assert(w);
    for (T* n = iterator() ; n ; n = wipe(n, w)) { }
    return *this;
  }

private:
  bucket_t * buckets_;
  size_t n_buckets_;
  lace::divider divider_;

  static bool is_bound(const T* n) { assert(n); return (n->*L).bound(); }

  size_t modulo(lace::hash_t h) const { return divider_.modulo(h, n_buckets_); }

  size_t index(const K & k) const { return modulo(H(k)); }
  size_t index(const T* n) const { assert(n); return index(n->*key); }

  bucket_t* is_bucket(const T* n) const {
    assert(n);
    const bucket_t* b = reinterpret_cast<const bucket_t*>(n);
    return (&buckets_[0] <= b && b < &buckets_[n_buckets_]) ? const_cast<bucket_t*>(b) : NULL;
  }

  static T* take_next(T ** p) {
    assert(p);
    assert(*p);

    T* t = *p;
    assert(is_bound(t));

    *p = (t->*L).p;
    (t->*L).p = NULL;

    assert(!is_bound(t));

    return t;
  }

  static T* insert_at(T ** p, T* t) {
    assert(p);
    assert(*p);

    assert(t);
    assert(t != *p);
    assert(!is_bound(t));

    (t->*L).p = *p;
    *p = t;

    assert(is_bound(t));

    return t;
  }

  void take_all(bucket_t & ts) {
    assert(!ts.p);
    ts.p = ts.sentinel();

    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      bucket_t & b = buckets_[i];
      while (b.p != b.sentinel())
        insert_at(&ts.p, take_next(&b.p));
    }
  }

  void give_all(bucket_t & ts) {
    assert(ts.p);

    while (ts.p != ts.sentinel())
      set(take_next(&ts.p));

    assert(ts.p == ts.sentinel());
    ts.p = NULL;
  }

  void set_buckets() {
    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      assert(!buckets_[i].p);
      buckets_[i].p = buckets_[i].sentinel();
    }
    assert(empty());
  }

  void reset_buckets() {
    assert(empty());
    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      assert(buckets_[i].p == buckets_[i].sentinel());
      buckets_[i].p = NULL;
    }
  }
};

} // namespace lite

#endif//LITE__TABLE_H
