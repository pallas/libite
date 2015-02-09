#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <sstream>
#include <string>

#include <algorithm>

#include <unistd.h>

#include "compare.h"
#include "intrusive_heap.h"
#include "intrusive_order.h"
#include "intrusive_table.h"

struct vertex_t;

struct edge_t {
  edge_t(vertex_t* f, vertex_t* t, unsigned w) : from(f), to(t), cost(w) { }

  vertex_t* from;
  vertex_t* to;
  unsigned cost;

  intrusive_order_link<edge_t> to_link;
  typedef intrusive_order<edge_t, &edge_t::to_link, typeof(edge_t::cost),
                          &edge_t::cost> to_edges_t;

  intrusive_order_link<edge_t> from_link;
  typedef intrusive_order<edge_t, &edge_t::from_link, typeof(edge_t::cost),
                          &edge_t::cost> from_edges_t;

  bool
  bound() const {
    return false
        || to_link.bound()
        || from_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }
};

struct vertex_t {
  vertex_t(const char * c)
    : id(strdup(c))
    , route(NULL)
  { }
  ~vertex_t() { free(const_cast<char*>(id)); }

  const char* id;
  unsigned cost;
  vertex_t* route;

  edge_t::to_edges_t to_edges;
  edge_t::from_edges_t from_edges;

  intrusive_table_link<vertex_t> link;
  typedef intrusive_table<vertex_t, &vertex_t::link, typeof(vertex_t::id),
                          &vertex_t::id> vertices_t;

  bool
  bound() const {
    return false
        || link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }
};

struct test_t {
  test_t(vertex_t* v, unsigned d) : vertex(v), cost(d) { }

  vertex_t* vertex;
  unsigned cost;

  intrusive_heap_link<test_t> link;
  typedef intrusive_heap<test_t, &test_t::link, typeof(test_t::cost),
                         &test_t::cost> pq_t;

  bool
  bound() const {
    return false
        || link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }
};

void
expand(vertex_t::vertices_t & vs, unsigned size) {
  delete [] vs.rehash(new vertex_t::vertices_t::bucket_t[size], size);
}

int
main(int, char* argv[]) {
  const unsigned load = 2;
  unsigned n_vertices = 0;
  vertex_t::vertices_t vertices;

  vertex_t* s = NULL;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty() || '#' == line[0])
      continue;

    std::stringstream ss(line);
    std::string from, to;
    unsigned cost;
    ss >> from >> to >> cost;

    vertex_t* f = vertices.get(from.c_str());
    if (!f) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        expand(vertices, (load+1) * (n_vertices+1));
      vertices.set(f = new vertex_t(from.c_str()));
    }
    if (!s || compare(f->id, s->id) < 0)
      s = f;

    vertex_t* t = vertices.get(to.c_str());
    if (!t) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        expand(vertices, (load+1) * (n_vertices+1));
      vertices.set(t = new vertex_t(to.c_str()));
    }
    if (!s || compare(t->id, s->id) < 0)
      s = t;

    edge_t* e = new edge_t(f, t, cost);
    f->from_edges.insert(e);
    t->to_edges.insert(e);
  }

  test_t::pq_t pq;
  if (s) {
    s->cost = 0;
    s->route = s;
    pq.inhume(new test_t(s, s->cost));
  }

  while (!pq.empty()) {
    test_t* t = pq.exhume();
    vertex_t* v = t->vertex;

    assert(v->route);
    edge_t::from_edges_t & fe = v->from_edges;
    for (edge_t* e = fe.iterator() ; e ; e = fe.next(e)) {
      assert(v == e->from);
      unsigned cost = e->from->cost + e->cost;
      if (!e->to->route || cost < e->to->cost) {
        e->to->cost = cost;
        e->to->route = e->from;
        pq.inhume(new test_t(e->to, e->to->cost));
      }
    }

    t->kill();
  }

  for (vertex_t* v = vertices.iterator() ; v ;
       std::cout << std::endl, v = vertices.next(v)) {
    std::cout << v->id;
    if (!v->route)
      continue;

    for (vertex_t* i = v ; s != i ; i = i->route)
      std::cout << " " << i->route->id;

    std::cout << " " << v->cost;
  }

  vertex_t* v = vertices.iterator();
  while (v) {
    while (!v->to_edges.empty())
      v->to_edges.remove()->kill();
    while (!v->from_edges.empty())
      v->from_edges.remove()->kill();
    vertex_t* x = vertices.next(v);
    std::swap(x, v);
    vertices.bus(x)->kill();
  }

  delete [] vertices.dehash();
  return EXIT_SUCCESS;
}

//
