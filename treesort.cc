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

  void axe() { if (!bound()) delete this; }
};

int
main(int, char*[]) {
  node::tree_t tree;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      tree.graft(new node(i));
  }

  for (node* i = tree.min() ; i ; i = tree.next(i))
    std::cout << i->value << std::endl;

  tree.fell(&node::axe);
  return EXIT_SUCCESS;
}

//
