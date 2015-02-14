#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "intrusive_order.h"

struct node {
  long value;
  node(int v) : value(v) { }

  intrusive_order_link<node> order_link;

  bool
  bound() const {
    return false
        || order_link.bound()
        ;;
  }

  typedef intrusive_order<node, &node::order_link, typeof(node::value), &node::value> order_t;
};

int
main(int, char*[]) {
  node::order_t order;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      order.insert(new node(i));
  }

  while (!order.empty()) {
    node* x = order.remove();
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
