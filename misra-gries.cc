#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "table.h"
#include "heap.h"

struct node {
  int value, weight;
  node (int v) : value(v), weight(0) { }

  lite::table_link<node> table_link;
  lite::heap_link<node> heap_link;

  bool
  bound() const {
    return false
        || table_link.bound()
        || heap_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::table<node, &node::table_link, typeof(node::value), &node::value> table_t;
  typedef lite::heap<node, &node::heap_link, typeof(node::weight), &node::weight> heap_t;
};

int
main(int, char*[]) {
  const unsigned k = 100;

  node::heap_t heap;

  node::table_t::bucket_t buckets[k];
  node::table_t table(buckets, sizeof(buckets)/sizeof(*buckets));

  unsigned entries = 0;
  unsigned count = 0;
  unsigned waterline = 0;
  while (std::cin) {
    int i;
    if (!(std::cin >> i).good())
      continue;

    ++count;
    unsigned weight = 1;
    if (node* x = table.get(i)) {
      x->weight += weight;
      heap.bury(x);
    } else if (entries < k) {
      ++entries;
      node* x = new node(i);
      x->weight = waterline + weight;
      table.set(x);
      heap.inhume(x);
    } else {
      waterline += weight;
      while (!heap.empty() && heap.root()->weight <= waterline) {
        --entries;
        table.bus(heap.exhume())->kill();
      }
    }

  }

  while (!heap.empty()) {
    node* x = heap.exhume();
    float p = float(x->weight - waterline)/(count - waterline);
    if (p * k >= 1)
      std::cout << p << '\t' << x->value << std::endl;
    table.bus(x)->kill();
  }

  table.dehash();

  return EXIT_SUCCESS;
}

//
