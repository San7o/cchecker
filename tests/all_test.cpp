// MIT License
//
// Copyright (c) 2024 Giovanni Santini
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define CHECK_DEBUG
#include "../cchecker.hpp"
#include "tests.hpp"
#include <cassert>

#include <cstdio>

void test_all()
{
  check::Val<int> x = 1;
  check::ValMut<int> y = 2;
  assert(x.get() == 1);
  assert(y.get() == 2);
  y.set(3);
  assert(y.get() == 3);

  // make a copy of an immutable
  check::ValMut<int> x2 = x;
  assert(x.get() == 1);
  assert(x2.get() == 1);
  
  // make a copy of a mutable
  check::ValMut<int> z = y;
  assert(y.get() == 3);
  assert(z.get() == 3);

  // multiple immutable references of an immutable
  check::ValRef<int> a = x.getRef();
  check::ValRef<int> a2 = x.getRef();
  assert(a.get() == 1);
  assert(a2.get() == 1);

  // multiple immutable refs of a mutable
  check::ValRef<int> b = y.getRef();
  check::ValRef<int> b2 = y.getRef();
  assert(b.get() == 3);
  assert(b2.get() == 3);
  //check::ValMutRef<int> b3 = y.getMutRef(); // FAIL!

  // only one mutable reference
  check::ValMut<int> c = 10;
  check::ValMutRef<int> d = c.getMutRef();
  // assert(c.get() == 10); // FAIL!
  assert(d.get() == 10);

  return;
}
