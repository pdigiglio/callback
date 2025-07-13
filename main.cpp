#include "callback.h"

#include <cstdio>

namespace {

struct S {
  int i;
  void f(int j) { std::printf("i : %d; j : %d\n", i, j); }
};

void f(int i) { std::printf("i : %d\n", i); }
} // namespace

int main() {
  {
    Callback<void(int)> cb(f);
    cb(0);
  }

  {
    S s;
    s.i = 1;

    Callback<void(int)> cb(s, &S::f);
    cb(0);
  }
}
