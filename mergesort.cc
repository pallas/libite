#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include <lace/haystack.h>

#include "queue.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::queue_link<node> queue_link;

  typedef lite::queue<node, &node::queue_link> queue_t;
  typedef queue_t::sorter<typeof(value), &node::value> sorter_t;
};

int
main(int, char*[]) {
  lace::haystack h;
  node::queue_t queue;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      queue.enqueue(new (h.allocate<node>()) node(i));
  }

  node::sorter_t::sort(queue);

  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << x->value << std::endl;
  }

  return EXIT_SUCCESS;
}

//
