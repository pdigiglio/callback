#include "callback.h"

#include <cstdio>

// Test:
//  1. void can discard return value
//  2. covariant return types
//  3. contravariant param types

namespace {

class PrintScope {
  char const *_scopeName;

public:
  PrintScope(char const *scopeName) //
      : _scopeName(scopeName) {
    std::printf("{ %s\n", _scopeName);
  }

  ~PrintScope() { //
    std::printf("} %s\n", _scopeName);
  }
};

#define PRINT_SCOPE PrintScope const _(__PRETTY_FUNCTION__) 

struct Base {
  virtual ~Base() {}

  void f_void() {}
  int f_int() { return 0; }

  Base const *f_c(Base const &base) const {
    PRINT_SCOPE;
    return &base;
  }

  virtual Base const *f(Base const &base) {
    PRINT_SCOPE;
    return f_c(base);
  }

  // virtual int vf_int() { return 0; }
};

struct Derived : Base {
  virtual Base const *f(Base const &derived) {
    PRINT_SCOPE;
    return &derived;
  }

  // virtual  int vf_int() { return 1; }
};

void foo(Base const &base) { //
  std::printf("Base at %p\n", reinterpret_cast<void const *>(&base));
}
long bar(Derived const &derived) {
  std::printf("Derived at %p\n", reinterpret_cast<void const *>(&derived));
  return 0;
}

struct S {
  int i;
  void f(int j) { std::printf("i : %d; j : %d\n", i, j); }
};

void f(int i) { std::printf("i : %d\n", i); }


// template <typename R,  >

} // namespace

int main() {
  {
    Derived d;
    Callback<void(Derived const &)> cb;

    {
      cb = Callback<void(Derived const &)>(foo);
      cb(d);
    }

    {
      cb = Callback<void(Derived const &)>(bar);
      cb(d);
    }

    {
      cb = Callback<void(Derived const &)>(d, &Base::f);
      cb(d);
    }

    {
      cb = Callback<void(Derived const &)>(static_cast<Base &>(d), &Base::f_c);
      cb(d);
    }
  }

  // {
  //   Callback<void(int)> cb(f);
  //   cb(0);
  // }
  //
  // {
  //   S s;
  //   s.i = 1;
  //
  //   Callback<void(int)> cb(s, &S::f);
  //   cb(0);
  // }
}
