#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "set.h"
#include "tree.h"
#include "heap.h"
#include "stack.h"
#include "queue.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::set_link<node> set_link;
  lite::tree_link<node> tree_link;
  lite::heap_link<node> heap_link;
  lite::queue_link<node> queue_link;
  lite::stack_link<node> stack_link;

  bool
  bound() const {
    return false
        || set_link.bound()
        || tree_link.bound()
        || heap_link.bound()
        || queue_link.bound()
        || stack_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::set<node, &node::set_link> set_t;
  typedef lite::tree<node, &node::tree_link, typeof(node::value), &node::value> tree_t;
  typedef lite::heap<node, &node::heap_link, typeof(node::value), &node::value> heap_t;
  typedef lite::queue<node, &node::queue_link> queue_t;
  typedef lite::stack<node, &node::stack_link> stack_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::tree_t tree;
  node::heap_t heap;
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

  node* y = new node(lrand48() % n);
  tree.graft(y);
  heap.inhume(y);
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

  {{
    node::heap_t h;
    for (node* i = odd.iterator() ; i ; i = odd.next(i)) {
      if (i->heap_link.bound()) {
        h.inhume(new node(i->value));
        heap.sift(i)->kill();
      }
    }
    heap.meld(h);
  }}

  std::cout << "heap" << '\t';
  for (node* i = tree.min() ; i ; i = tree.next(i)) {
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
