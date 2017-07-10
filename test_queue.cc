#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include "queue.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::queue_link<node> forward_link, reverse_link;

  bool
  bound() const {
    return false
        || forward_link.bound()
        || reverse_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::queue<node, &node::forward_link> forward_queue_t;
  typedef lite::queue<node, &node::reverse_link> reverse_queue_t;

  typedef forward_queue_t::sorter<typeof(value), &node::value, lace::compare<typeof(value)> > sorter_t;
};

int
main(int, char*[]) {
  srand48(getpid());

  node::forward_queue_t forward;
  node::reverse_queue_t reverse;

  static const unsigned n = 16;

  for (unsigned i = 0 ; i < n ; ++i) {
    node* x = new node(lrand48() % n);
    forward.enqueue(x);
  }

  node::sorter_t::sort(forward);

  for (node * n = forward.iterator() ; n ; n = forward.next(n))
    reverse.enqueue(n);

  reverse.reverse();

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
