#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "intrusive_heap.h"

struct node {
  long value;
  node(int v) : value(v) { }

  intrusive_heap_link<node> heap_link;

  bool
  bound() const {
    return false
        || heap_link.bound()
        ;;
  }

  typedef intrusive_heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
};

int
main(int, char*[]) {
  node::heap_t heap;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      heap.inhume(new node(i));
  }

  while (!heap.empty()) {
    node* x = heap.exhume();
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
