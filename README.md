libite
======

libite is a set of header-based intrusive containers in C++, originally part
of [liboco](https://github.com/pallas/liboco) and created as an exercise for
the author.  It is not meant as a replacement for other, similar template
libraries.

The following container templates are currently implemented

 * intrusive_order --- Sorted container implementing insert & delete with
   optimized inserts at the back as well as efficient merge.
 * intrusive_queue --- FIFO container implementing enqueue & dequeue.
 * intrusive_stack --- LIFO container implementing push & pop.
 * intrusive_tree --- Red-black tree implementing graft & prune, as well as
   find and efficient iteration via next & prev.  With assertions enabled,
   the entire tree is validated as pre- and post-conditions; to disable,
   define NDEBUG.

as well as the following utility templates

 * do_not_copy --- Equivalent to boost::noncopyable.
 * intrusive_link --- Shared pointer wrapper, which asserts that links are
   removed from a container prior to object destruction.

A small test suite is available and can be run via

 * make test

While libite is just a toy, the intention is to continue to add features and
new containers.
