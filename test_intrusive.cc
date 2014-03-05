#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "intrusive_tree.h"
#include "intrusive_stack.h"
#include "intrusive_queue.h"
#include "intrusive_order.h"

struct node {
  int value;
  node(int v) : value(v) { }

  intrusive_tree_link<node> tree_link;
  intrusive_order_link<node> order_link;
  intrusive_queue_link<node> queue_link;
  intrusive_stack_link<node> stack_link;

  bool
  bound() const {
    return false
        || tree_link.bound()
        || order_link.bound()
        || queue_link.bound()
        || stack_link.bound()
        ;;
  }

  typedef intrusive_tree<node, &node::tree_link, typeof(node::value), &node::value> tree_t;
  typedef intrusive_order<node, &node::order_link, typeof(node::value), &node::value> order_t;
  typedef intrusive_queue<node, &node::queue_link> queue_t;
  typedef intrusive_stack<node, &node::stack_link> stack_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::tree_t tree;
  node::order_t even, odd, all;
  node::queue_t queue;
  node::stack_t stack;

  static const unsigned n = 16;
  for (unsigned i = 0 ; i < n ; ++i) {
    node* x = new node(lrand48() % n);

    tree.graft(x);

    switch (x->value % 2) {
    case 0: even.insert(x); break;
    case 1: odd.insert(x); break;
    }

    queue.enqueue(x);
    stack.push(x);
  }

  all.merge(even).merge(odd);

  std::cout << "order";
  for (node* i = tree.min() ; i ; i = tree.next(i)) {
    node* x = all.remove();
    assert(x->value == i->value);
    std::cout << ' ' << x->value;

    if (!x->bound())
      delete x;
  }
  std::cout << std::endl;

  std::cout << "queue";
  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << ' ' << tree.find(x->value)->value;

    assert(tree.is_member(x));
    tree.prune(x);

    if (!x->bound())
      delete x;
  }
  std::cout << std::endl;

  std::cout << "stack";
  while (!stack.empty()) {
    node* x = stack.pop();
    std::cout << ' ' << x->value;

    if (!x->bound())
      delete x;
  }
  std::cout << std::endl;

  return EXIT_SUCCESS;
}

//
