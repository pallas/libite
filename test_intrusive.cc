#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "intrusive_set.h"
#include "intrusive_tree.h"
#include "intrusive_heap.h"
#include "intrusive_stack.h"
#include "intrusive_queue.h"
#include "intrusive_order.h"

struct node {
  int value;
  node(int v) : value(v) { }

  intrusive_set_link<node> set_link;
  intrusive_tree_link<node> tree_link;
  intrusive_heap_link<node> heap_link;
  intrusive_order_link<node> order_link;
  intrusive_queue_link<node> queue_link;
  intrusive_stack_link<node> stack_link;

  bool
  bound() const {
    return false
        || set_link.bound()
        || tree_link.bound()
        || heap_link.bound()
        || order_link.bound()
        || queue_link.bound()
        || stack_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef intrusive_set<node, &node::set_link> set_t;
  typedef intrusive_tree<node, &node::tree_link, typeof(node::value), &node::value> tree_t;
  typedef intrusive_heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
  typedef intrusive_order<node, &node::order_link, typeof(node::value), &node::value> order_t;
  typedef intrusive_queue<node, &node::queue_link> queue_t;
  typedef intrusive_stack<node, &node::stack_link> stack_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::tree_t tree;
  node::heap_t heap;
  node::order_t order;
  node::queue_t queue;
  node::stack_t stack;

  static const unsigned n = 16;

  {{
    node::tree_t even, odd;

    for (unsigned i = 0 ; i < n ; ++i) {
      node* x = new node(lrand48() % n);

      switch (x->value % 2) {
      case 0: even.graft(x); break;
      case 1: odd.graft(x); break;
      }

      queue.enqueue(x);
      stack.push(x);
      heap.inhume(x);
    }

    tree.inosculate(even).inosculate(odd);
  }}

  {{
    node::order_t even, odd;

    for (node* x = tree.min() ; x ; x = tree.next(x)) {
      switch (x->value % 2) {
      case 0: even.insert(x); break;
      case 1: odd.insert(x); break;
      }
    }

    order.merge(even).merge(odd);
  }}

  node* y = new node(lrand48() % n);
  tree.graft(y);
  heap.inhume(y);
  order.insert(y);
  queue.enqueue(y);
  stack.push(y);

  std::cout << "xyzzy" << '\t';
  for (node* i = heap.root() ; i ; i = heap.next(i)) {
    std::cout << ' ' << i->value;
    if (i == y)
        std::cout << '*';
  }
  std::cout << std::endl;

  {{
    node* z = new node(y->value);
    tree.transplant(y, z);
    assert(z->bound());
    y->kill();
  }}

  node::set_t even, odd;

  std::cout << "tree" << '\t';
  for (node* i = tree.min() ; i ; i = tree.next(i)) {
    std::cout << ' ' << i->value;
    if (i == y)
        std::cout << '*';
    switch (i->value % 2) {
    case 0: even.join(i); break;
    case 1: odd.join(i); break;
    }
  }
  std::cout << std::endl;

  for (node* i = odd.iterator() ; i ; i = odd.next(i)) {
    if (i->heap_link.bound()) {
      heap.inhume(new node(i->value));
      heap.sift(i)->kill();
    }
  }

  std::cout << "heap" << '\t';
  for (node* i = order.iterator() ; i ; i = order.next(i)) {
    node* x = heap.exhume();
    assert(x->value == i->value);
    std::cout << ' ' << x->value;
    if (x == y)
        std::cout << '*';

    if (node::set_t::typed(x))
      switch (x->value % 2) {
      case 0: assert(even.contains(x)); break;
      case 1: assert(odd.contains(x)); break;
      }

    x->kill();
  }
  std::cout << std::endl;

  node::set_t all;

  all.conjoin(&even);
  assert(even.empty());

  all.conjoin(&odd);
  assert(odd.empty());

  std::cout << "set" << '\t';
  for (node* i = all.iterator() ; i ; i = all.next(i)) {
    std::cout << ' ' << i->value;
    if (i == y)
        std::cout << '*';
  }
  std::cout << std::endl;

  all.dissolve(&node::kill);

  std::cout << "order" << '\t';
  for (node* i = tree.min() ; i ; i = tree.next(i)) {
    node* x = order.remove();
    assert(x->value == i->value);
    std::cout << ' ' << x->value;
    if (x == y)
        std::cout << '*';

    x->kill();
  }
  std::cout << std::endl;

  std::cout << "queue" << '\t';
  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << ' ' << tree.find(x->value)->value;
    if (x == y)
        std::cout << '*';

    assert(tree.is_member(x) || x == y);
    if (x != y)
      tree.prune(x);

    x->kill();
  }
  std::cout << std::endl;

  std::cout << "stack" << '\t';
  while (!stack.empty()) {
    node* x = stack.pop();
    std::cout << ' ' << x->value;
    if (x == y)
        std::cout << '*';

    x->kill();
  }
  std::cout << std::endl;

  {{
    node* z = tree.prune(tree.root());
    z->kill();
  }}

  return EXIT_SUCCESS;
}

//
