#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "intrusive_queue.h"

struct node {
  long value;
  node(int v) : value(v) { }

  intrusive_queue_link<node> queue_link;

  bool
  bound() const {
    return false
        || queue_link.bound()
        ;;
  }

  typedef intrusive_queue<node, &node::queue_link> queue_t;
};

int
main(int, char*[]) {
  node::queue_t queue;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      queue.enqueue(new node(i));
  }

  queue.sort<typeof(node::value), &node::value>();

  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
