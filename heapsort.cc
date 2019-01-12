#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include <lace/haystack.h>

#include "heap.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::heap_link<node> heap_link;
  typedef lite::heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
};

int
main(int, char*[]) {
  lace::haystack h;
  node::heap_t heap;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      heap.inhume(new (h.allocate<node>()) node(i));
  }

  while (!heap.empty()) {
    node* x = heap.exhume();
    std::cout << x->value << std::endl;
  }

  return EXIT_SUCCESS;
}

//
