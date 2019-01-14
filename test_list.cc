#include <cstdlib>
#include <cassert>
#include <iostream>

#include <unistd.h>

#include <lace/singleton.h>
#include <lace/random.h>
#include "queue.h"
#include "list.h"

struct node {
  int value;
  node(int v) : value(v) { }

  lite::queue_link<node> queue_link;
  lite::list_link<node> list_link;

  bool
  bound() const {
    return false
        || queue_link.bound()
        || list_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  typedef lite::queue<node, &node::queue_link> queue_t;
  typedef lite::list<node, &node::list_link> list_t;

  typedef queue_t::sorter<typeof(value), &node::value> sorter_t;
};

int
main(int, char*[]) {
  lace::random & rng = lace::singleton<lace::random>::instance();

  node::queue_t queue;
  node::list_t list;

  static const unsigned n = 16;

  for (unsigned i = 0 ; i < n ; ++i) {
    node* x = new node(rng.l() % n);
    list.enlist(x);
    queue.enqueue(x);
  }

  {{
    node* t = new node(n);
    for (node * n = list.last() ; n ; n = list.previous(n))
      list.enlist(t, n).delist(t);

    std::cout << "ring" << '\t';
    for (node * n = list.enlist(t).first() ; n != t ; n = list.prograde()) {
      assert(n == list.first());
      std::cout << ' ' << n->value;
    }
    std::cout << std::endl;

    list.delist(t)->kill();
  }}

  node::sorter_t::sort(queue);

  std::cout << "list" << '\t';
  for (node * n = list.first() ; n ; n = list.next(n))
      std::cout << ' ' << n->value;
  std::cout << std::endl;

  std::cout << "queue" << '\t';
  while (!queue.empty()) {
    node* x = queue.dequeue();
    std::cout << ' ' << x->value;
    list.delist(x);
    x->kill();
  }
  std::cout << std::endl;

  return EXIT_SUCCESS;
}

//
