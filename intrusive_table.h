#ifndef INTRUSIVE_TABLE_H
#define INTRUSIVE_TABLE_H

#include "do_not_copy.h"
#include "intrusive_link.h"

#include "hash.h"
#include "compare.h"

#include <cassert>
#include <cstddef>
#include <algorithm>

template <class X>
struct intrusive_table_link : private intrusive_link<X> {
  typedef intrusive_table_link type;
  template <class T, typename intrusive_table_link<T>::type T::*link,
            typename K, K T::*key, compare_t (*C)(K const &, K const &),
            hash_t (*H)(K const &)>
    friend class intrusive_table;

  bool bound() const { return intrusive_link<X>::p; }
};

template <class X>
class intrusive_table_bucket : private intrusive_link<X> {
  template <class T, typename intrusive_table_link<T>::type T::*link,
            typename K, K T::*key, compare_t (*C)(K const &, K const &),
            hash_t (*H)(K const &)>
    friend class intrusive_table;
private:
  X* sentinal() { return reinterpret_cast<X*>(this); }
  const X* sentinal() const { return reinterpret_cast<const X*>(this); }
};

template <class T, typename intrusive_table_link<T>::type T::*link,
          typename K, K T::*key,
          compare_t (*C)(K const &, K const &) = compare<K>,
          hash_t (*H)(K const &) = hash<K> >
class intrusive_table : public do_not_copy {
public:
  typedef intrusive_table_bucket<T> bucket_t;

  intrusive_table(bucket_t bs[] = NULL, const size_t n = 0)
    : buckets_(bs), n_buckets_(n)
  { assert(buckets_ || 0 == n_buckets_); set_buckets(); }

  ~intrusive_table() { assert(empty()); reset_buckets(); }

  bool empty() const {
    assert(buckets_ || 0 == n_buckets_);

    for (size_t i = 0 ; i < n_buckets_ ; ++i)
      if (buckets_[i].p != buckets_[i].sentinal())
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

    set_buckets();
    give_all(ts);

    return bs;
  }

  bucket_t* dehash() {
    assert(empty());
    return rehash(NULL, 0);
  }

  intrusive_table & set(T* t) {
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
      c = &((*c)->*link).p;

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
    bucket_t & b = buckets_[index(k)];

    for (T ** c = &b.p ; *c != b.sentinal() ; c = &((*c)->*link).p)
      if (0 == C(k, (*c)->*key))
          return c == &b.p ? *c : insert_at(&b.p, take_next(c));

    return NULL;
  }

  bool is_member(const T* t) const {
    bucket_t & b = buckets_[index(t)];

    for (T ** c = &b.p ; *c != b.sentinal() ; c = &((*c)->*link).p)
      if (t == *c)
        return true;

    return false;
  }

  T* iterator() const {
    for (size_t i = 0 ; i < n_buckets_ ; ++i)
      if (buckets_[i].p != buckets_[i].sentinal())
        return buckets_[i].p;

    return NULL;
  }

  T* next(const T* t) const {
    assert(is_member(t));

    T* n = (t->*link).p;

    if (bucket_t* b = is_bucket(n)) {
      size_t o = b - buckets_;
      for (size_t i = o + 1; i < n_buckets_ ; ++i)
        if (buckets_[i].p != buckets_[i].sentinal())
          return buckets_[i].p;

      return NULL;
    }

    return n;
  }

private:
  bucket_t * buckets_;
  size_t n_buckets_;

  static bool is_bound(const T* n) { assert(n); return (n->*link).bound(); }

  hash_t index(const K & k) const { return H(k) % n_buckets_; }
  hash_t index(const T* n) const { assert(n); return index(n->*key); }

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

    *p = (t->*link).p;
    (t->*link).p = NULL;

    assert(!is_bound(t));

    return t;
  }

  static T* insert_at(T ** p, T* t) {
    assert(p);
    assert(*p);

    assert(t);
    assert(t != *p);
    assert(!is_bound(t));

    (t->*link).p = *p;
    *p = t;

    assert(is_bound(t));

    return t;
  }

  void take_all(bucket_t & ts) {
    assert(!ts.p);
    ts.p = ts.sentinal();

    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      bucket_t & b = buckets_[i];
      while (b.p != b.sentinal())
        insert_at(&ts.p, take_next(&b.p));
    }
  }

  void give_all(bucket_t & ts) {
    assert(ts.p);

    while (ts.p != ts.sentinal())
      set(take_next(&ts.p));

    assert(ts.p == ts.sentinal());
    ts.p = NULL;
  }

  void set_buckets() {
    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      assert(!buckets_[i].p);
      buckets_[i].p = buckets_[i].sentinal();
    }
    assert(empty());
  }

  void reset_buckets() {
    assert(empty());
    for (size_t i = 0 ; i < n_buckets_ ; ++i) {
      assert(buckets_[i].p == buckets_[i].sentinal());
      buckets_[i].p = NULL;
    }
  }
};

#endif//INTRUSIVE_TABLE_H
