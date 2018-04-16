#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <sstream>
#include <string>

#include <algorithm>

#include <unistd.h>

#include <lace/compare.h>
#include "heap.h"
#include "queue.h"
#include "table.h"

struct vertex_t;

struct edge_t {
  edge_t(vertex_t* f, vertex_t* t, unsigned w) : from(f), to(t), cost(w) { }

  vertex_t* from;
  vertex_t* to;
  unsigned cost;

  lite::queue_link<edge_t> from_link;
  typedef lite::queue<edge_t, &edge_t::from_link> from_edges_t;
  typedef from_edges_t::sorter<typeof(edge_t::cost), &edge_t::cost> from_edges_sorter;

  bool
  bound() const {
    return false
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

  edge_t::from_edges_t from_edges;

  lite::table_link<vertex_t> v_link;
  typedef lite::table<vertex_t, &vertex_t::v_link, typeof(vertex_t::id), &vertex_t::id> vertices_t;

  lite::heap_link<vertex_t> q_link;
  typedef lite::heap<vertex_t, &vertex_t::q_link, typeof(vertex_t::cost), &vertex_t::cost> pq_t;

  bool
  bound() const {
    return false
        || v_link.bound()
        || q_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }
};

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
    unsigned cost = 1;
    ss >> from >> to >> cost;

    vertex_t* f = vertices.get(from.c_str());
    if (!f) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        vertices.reseat((load+1) * (n_vertices+1));
      vertices.set(f = new vertex_t(from.c_str()));
    }
    if (!s || lace::compare(f->id, s->id) < 0)
      s = f;

    vertex_t* t = vertices.get(to.c_str());
    if (!t) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        vertices.reseat((load+1) * (n_vertices+1));
      vertices.set(t = new vertex_t(to.c_str()));
    }
    if (!s || lace::compare(t->id, s->id) < 0)
      s = t;

    edge_t* e = new edge_t(f, t, cost);
    f->from_edges.enqueue(e);
  }

  for (vertex_t* v = vertices.iterator() ; v ; v = vertices.next(v))
    edge_t::from_edges_sorter::sort(v->from_edges);

  vertex_t::pq_t pq;
  if (s) {
    s->cost = 0;
    s->route = s;
    pq.inhume(s);
  }

  while (!pq.empty()) {
    vertex_t* v = pq.exhume();

    assert(v->route);
    edge_t::from_edges_t & fe = v->from_edges;
    for (edge_t* e = fe.iterator() ; e ; e = fe.next(e)) {
      assert(v == e->from);
      unsigned cost = e->from->cost + e->cost;
      if (!e->to->route || cost < e->to->cost) {
        e->to->cost = cost;
        e->to->route = e->from;
        pq.churn(e->to);
      }
    }
  }

  for (vertex_t* v = vertices.iterator() ; v ; v = vertices.next(v)) {
    if (!v->route)
      continue;

    std::cout << v->id << " "
      << v->route->id << " "
      << v->cost << std::endl;
  }

  vertex_t* v = vertices.iterator();
  while (v) {
    while (!v->from_edges.empty())
      v->from_edges.dequeue()->kill();
    vertex_t* x = vertices.next(v);
    std::swap(x, v);
    vertices.bus(x)->kill();
  }

  vertices.reseat();
  return EXIT_SUCCESS;
}

//
