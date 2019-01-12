#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include <lace/haystack.h>

#include "tree.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::tree_link<node> tree_link;
  typedef lite::tree<node, &node::tree_link, typeof(node::value), &node::value> tree_t;
};

int
main(int, char*[]) {
  lace::haystack h;
  node::tree_t tree;

  while (std::cin) {
    int i;
    if ((std::cin >> i).good())
      tree.graft(new (h.allocate<node>()) node(i));
  }

  if (!tree.empty())
    for (node* i = tree.min() ; i ; i = tree.next(i))
      std::cout << i->value << std::endl;

#ifndef NDEBUG
  tree.fell();
#endif

  return EXIT_SUCCESS;
}

//
