#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "tree.h"

struct node {
  long value;
  node(int v) : value(v) { }

  lite::tree_link<node> tree_link;

  bool
  bound() const {
    return false
        || tree_link.bound()
        ;;
  }

  typedef lite::tree<node, &node::tree_link, typeof(node::value), &node::value> tree_t;
};

int
main(int, char*[]) {
  node::tree_t tree;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      tree.graft(new node(i));
  }

  while (!tree.empty()) {
    node* x = tree.prune(tree.min());
    std::cout << x->value << std::endl;
    if (!x->bound())
      delete x;
  }

  return EXIT_SUCCESS;
}

//
