#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "heap.h"

struct node {
  const char * value;
  node(const char * v) : value(v) { }

  lite::heap_link<node> heap_link;

  bool
  bound() const {
    return false
        || heap_link.bound()
        ;;
  }

  typedef lite::heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
};

int
main(int argc, char* argv[]) {
  node::heap_t heap;

  for (int i = 1 ; i < argc ; ++i)
    heap.inhume(new node(argv[i]));

  while (!heap.empty()) {
    node* x = heap.exhume();
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
