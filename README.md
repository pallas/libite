libite
======

libite is a set of header-based intrusive containers in C++, originally part
of [liboco](https://github.com/pallas/liboco) and created as an exercise for
the author.  It is not meant as a replacement for other, similar template
libraries.

The following container templates are currently implemented

 * intrusive_heap --- Pairing heap implementing inhume & exhume.
 * intrusive_order --- Sorted container implementing insert & delete with
   optimized inserts at the back as well as efficient merge.
 * intrusive_queue --- FIFO container implementing enqueue & dequeue.
 * intrusive_set --- Disjoint set implementing join, archetype, & dissolve.
 * intrusive_stack --- LIFO container implementing push & pop.
 * intrusive_table --- Chained hash table implementing set, get, & bus; user
   is responsible for providing hash buckets and performing rehashing.
 * intrusive_tree --- Red-black tree implementing graft & prune, as well as
   find and efficient iteration via next & prev.

as well as the following utility templates

 * compare --- Generic less-than, equal, greater-than template.
 * do_not_copy --- Equivalent to boost::noncopyable.
 * intrusive_link --- Shared pointer wrapper, which asserts that links are
   removed from a container prior to object destruction.
 * hash --- Generic hash template, implemented as 64-bit FNV-1a.
 * singleton --- Singleton template, instantiates before main.
 * try --- Converts traditional error reporting to exceptions.

and the following test programs

 * argsort --- Take program arguments, sort via heap, & write to std::cout.
 * bagofwords --- Take program arguments & write frequency to std::cout.
 * dijkstra --- Read (from, to, cost) tuples on std::cin, perform Dijkstra's
   algorithm on the graph starting at the lexicographically first node, &
   write routes plus costs to std::cout.
 * heapsort --- Read ints on std::cin, sort via heap, & write to std::cout.
 * kruskal --- Read (from, to, cost) tuples on std::cin, perform Kruskal's
   algorithm on the graph, & write edges composing a minimum spanning tree
   to std::cout.
 * treesort --- Read ints on std::cin, sort via tree, & write to std::cout.

With assertions enabled, intrusive_heap and intrusive_tree are validated as
pre- and post-conditions, which will wreck the algorithmic complexity; to
disable, define NDEBUG.

A small test suite is available and can be run via

 * make test

While libite is just a toy, the intention is to continue to add features and
new containers.
