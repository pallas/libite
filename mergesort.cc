#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "queue.h"

struct node {
  long value;
  node(int v) : value(v) { }

  lite::queue_link<node> queue_link;

  bool
  bound() const {
    return false
        || queue_link.bound()
        ;;
  }

  typedef lite::queue<node, &node::queue_link> queue_t;
  typedef queue_t::sorter<typeof(value), &node::value> sorter_t;
};

int
main(int, char*[]) {
  node::queue_t queue;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      queue.enqueue(new node(i));
  }

  node::sorter_t::sort(queue);

  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
