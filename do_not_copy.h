#ifndef DO_NOT_COPY_H
#define DO_NOT_COPY_H

class do_not_copy {
public:
  do_not_copy() { }

private:
  do_not_copy(const do_not_copy &);
  do_not_copy& operator=(const do_not_copy &);
};

#endif//DO_NOT_COPY_H
