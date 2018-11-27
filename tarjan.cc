#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <sstream>
#include <string>

#include <algorithm>

#include <unistd.h>

#include "set.h"
#include "heap.h"
#include "table.h"
#include "stack.h"

struct vertex_t;

struct edge_t {
  edge_t(vertex_t* f, vertex_t* t) : from(f), to(t) { }

  vertex_t* from;
  vertex_t* to;

  lite::stack_link<edge_t> from_link;
  typedef lite::stack<edge_t, &edge_t::from_link> from_edges_t;

  bool
  bound() const {
    return false
        || from_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }
};

struct vertex_t {
  vertex_t(const char * c) : id(strdup(c)), index(0), root(NULL), edge(NULL) { }
  ~vertex_t() { free(const_cast<char*>(id)); }

  const char* id;
  unsigned index;
  vertex_t* root;
  edge_t* edge;

  edge_t::from_edges_t from_edges;

  lite::table_link<vertex_t> table_link;
  typedef lite::table<vertex_t, &vertex_t::table_link, typeof(vertex_t::id), &vertex_t::id> table_t;

  lite::stack_link<vertex_t> stack_link;
  typedef lite::stack<vertex_t, &vertex_t::stack_link> stack_t;

  lite::set_link<vertex_t> set_link;
  typedef lite::set<vertex_t, &vertex_t::set_link> set_t;

  bool typed() const { return set_t::typed(this); }
  set_t* archetype() const { return set_t::archetype(this); }

  bool
  bound() const {
    return false
        || table_link.bound()
        || stack_link.bound()
        || set_link.bound()
        ;;
  }

  void kill() { if (!bound()) delete this; }

  bool unify(vertex_t* that) {
    if (this == that)
      return false;
    else if (!typed() && !that->typed())
      (new set_t)->join(this).join(that);
    else if (!typed())
      set_t::archetype(that)->join(this);
    else if (!that->typed())
      set_t::archetype(this)->join(that);
    else {
      set_t* this_a = set_t::archetype(this);
      set_t* that_a = set_t::archetype(that);
      if (this_a == that_a)
        return false;
      delete this_a->conjoin(that_a);
    }

    return true;
  }
};

int
main(int, char* argv[]) {
  const unsigned load = 2;
  unsigned n_vertices = 0;

  vertex_t::table_t vertices;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty() || '#' == line[0])
      continue;

    std::stringstream ss(line);
    std::string from, to;
    ss >> from >> to;

    vertex_t* f = vertices.get(from.c_str());
    if (!f) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        vertices.reseat((load+1) * (n_vertices+1));
      vertices.set(f = new vertex_t(from.c_str()));
    }

    vertex_t* t = vertices.get(to.c_str());
    if (!t) {
      ++n_vertices;
      if (vertices.buckets() < load * n_vertices)
        vertices.reseat((load+1) * (n_vertices+1));
      vertices.set(t = new vertex_t(to.c_str()));
    }

    f->from_edges.push(new edge_t(f, t));
  }

  unsigned count = 0;
  for (vertex_t* i = vertices.iterator() ; i ; i = vertices.next(i)) {
    if (i->root)
      continue;

    vertex_t::stack_t path;
    path.push(i);

    while (!path.empty()) {
      vertex_t* v = path.peek();

      edge_t::from_edges_t & fe = v->from_edges;

      if (!v->root) {
        v->root = v;
        v->index = ++count;
        v->edge = fe.iterator();
      } else if (v->edge) {
          assert(v->edge->from == v);

          if (!v->edge->to->index) {
            assert(!v->edge->to->stack_link.bound());
            path.push(v->edge->to);
          }

          v->edge = fe.next(v->edge);
      } else {
        path.pop();
        assert(v->root == v);

        for (edge_t* e = fe.iterator() ; e ; e = fe.next(e)) {
          assert(e->from == v);
          assert(e->to->index);
          assert(e->to->root);


          if (!e->to->root->stack_link.bound())
            continue;

          if (v->root->index <= e->to->root->index)
            continue;

          v->root = e->to->root;
          e->to->root->unify(v);
        }

        if (v->root == v) {
          if (v->typed()) {
            vertex_t::set_t* s = v->archetype();
            vertex_t* w = s->iterator();

            std::cout << w->id;
            while ((w = s->next(w)))
              std::cout << ' ' << w->id;

            std::cout << std::endl;
          } else
            std::cout << v->id << std::endl;
        }
      }
    }
  }

  for (vertex_t* v = vertices.iterator() ; v ;
       v = vertices.wipe(v, &vertex_t::kill)) {
    while (!v->from_edges.empty())
      v->from_edges.pop()->kill();
    if (v->typed())
      delete &v->archetype()->dissolve();
  }

  vertices.reseat();
  return EXIT_SUCCESS;
}

//
