#include <cstdlib>
#include <cassert>
#include <cmath>
#include <iostream>

#include <unistd.h>

#include "intrusive_heap.h"
#include "intrusive_table.h"

struct node {
  unsigned count;
  const char * string;
  node(const char * s) : count(0), string(s) { }

  intrusive_heap_link<node> heap_link;
  intrusive_table_link<node> table_link;

  bool
  bound() const {
    return false
        || table_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef intrusive_heap<node, &node::heap_link, typeof(node::count), &node::count, lace::reverse_compare<typeof(node::count)> > heap_t;
  typedef intrusive_table<node, &node::table_link, typeof(node::string), &node::string> table_t;
};

int
main(int argc, char* argv[]) {
  node::table_t::bucket_t buckets[unsigned(sqrt(argc))];
  node::table_t table(buckets, sizeof(buckets)/sizeof(*buckets));

  for (int i = 1 ; i < argc ; ++i) {
    node* x = table.get(argv[i]);
    if (!x)
      table.set(x = new node(argv[i]));
    ++x->count;
  }

  node::heap_t heap;
  for (node* x = table.iterator() ; x ; x = table.next(x))
    heap.inhume(x);

  while (!heap.empty()) {
    node* x = table.bus(heap.exhume());
    std::cout << x->count << " " << x->string << std::endl;
    x->kill();
  }

  return EXIT_SUCCESS;
}

//
