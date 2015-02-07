#ifndef HASH_H
#define HASH_H

#include <cstring>
#include <stdint.h>

typedef uint64_t hash_t;
typedef uint_fast64_t fast_hash_t;

namespace {
  hash_t
  fnv_1a(const unsigned char * d, size_t l) {
    fast_hash_t h = 0xcbf29ce484222325ULL;
    while (l--) {
      h ^= *d++;
      h *= 0x100000001b3ULL;
    }
    return h;
  }
};

template <typename T> hash_t hash(T const &t) {
  return fnv_1a(reinterpret_cast<const unsigned char *>(&t), sizeof(t));
}

template <> hash_t hash(const char * const &t) {
  return fnv_1a(reinterpret_cast<const unsigned char *>(t), strlen(t));
}

#endif//HASH_H
