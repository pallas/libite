#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "queue.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::queue_link<node> random_link, forward_link, reverse_link;

  bool
  bound() const {
    return false
        || forward_link.bound()
        || reverse_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::queue<node, &node::random_link> random_queue_t;
  typedef lite::queue<node, &node::forward_link> forward_queue_t;
  typedef lite::queue<node, &node::reverse_link> reverse_queue_t;

  typedef forward_queue_t::sorter<typeof(value), &node::value> forward_sorter_t;
  typedef reverse_queue_t::reverse_sorter<typeof(value), &node::value> reverse_sorter_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::random_queue_t random;
  node::forward_queue_t forward;
  node::reverse_queue_t reverse;

  static const unsigned n = 16;

  for (unsigned i = 0 ; i < n ; ++i) {
    node* x = new node(lrand48() % n);
    random.enqueue(x);
    forward.enqueue(x);
  }

  node::forward_sorter_t::sort(forward);

  for (node * n = forward.iterator() ; n ; n = forward.next(n))
    reverse.enqueue(n);

  reverse.reverse();
  assert(node::reverse_sorter_t::sorted(reverse));
  assert(forward.peek()->value == reverse.last()->value);
  assert(forward.last()->value == reverse.peek()->value);

  std::cout << "random" << '\t';
  while (!random.empty()) {
    node* x = random.dequeue();
    std::cout << ' ' << x->value;
    x->kill();
  }
  std::cout << std::endl;

  std::cout << "forward" << '\t';
  while (!forward.empty()) {
    node* x = forward.dequeue();
    std::cout << ' ' << x->value;
    x->kill();
  }
  std::cout << std::endl;

  std::cout << "reverse" << '\t';
  while (!reverse.empty()) {
    node* x = reverse.dequeue();
    std::cout << ' ' << x->value;
    x->kill();
  }
  std::cout << std::endl;

  reverse.reverse();

  return EXIT_SUCCESS;
}

//
