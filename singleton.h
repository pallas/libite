#ifndef SINGLETON_H
#define SINGLETON_H

#include "do_not_copy.h"

template <class T>
class singleton : public do_not_copy {
public:
  static T & instance() {
    instantiate();
    static T t;
    return t;
  }

private:
  static struct instantiator {
    instantiator() { singleton::instance(); }
    void operator()() { };
  } instantiate;
};

template <class T> struct singleton<T>::instantiator singleton<T>::instantiate;

#endif//SINGLETON_H
