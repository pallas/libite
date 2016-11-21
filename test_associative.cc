#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "heap.h"
#include "table.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::heap_link<node> heap_link;
  lite::table_link<node> table_link;

  bool
  bound() const {
    return false
        || heap_link.bound()
        || table_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
  typedef lite::table<node, &node::table_link, typeof(node::value), &node::value> table_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::heap_t heap;
  node::table_t table;

  const size_t BUCKETS = 8;
  node::table_t::bucket_t buckets[BUCKETS];
  table.rehash(buckets, BUCKETS/2);

  node* v = new node(lrand48() % 1000);
  table.set(v);

  static const unsigned n = 16;
  for (unsigned i = 0 ; i < n ; ++i) {
    node* x = new node(lrand48() % 1000);
    table.set(x);
    heap.inhume(x);
  }

  std::cout << "values";
  for (node* i = table.iterator() ; i ; i = table.next(i)) {
    std::cout << " " << i->value;
    if (i == v)
      std::cout << "*";
  }
  std::cout << std::endl;

  table[v->value];

  std::cout << "access";
  for (node* i = table.iterator() ; i ; i = table.next(i)) {
    std::cout << " " << i->value;
    if (i == v)
      std::cout << "*";
  }
  std::cout << std::endl;

  table.rehash(buckets, BUCKETS);

  std::cout << "rehash";
  for (node* i = table.iterator() ; i ; i = table.next(i)) {
    std::cout << " " << i->value;
    if (i == v)
      std::cout << "*";
  }
  std::cout << std::endl;

  while (!heap.empty())
    table.bus(heap.exhume())->kill();

  table.bus(table.get(v->value))->kill();
  table.dehash();

  return EXIT_SUCCESS;
}

//
