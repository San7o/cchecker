# cchecker

## TLDR

This [header only](./cchecker.hpp) library is a little run-time borrow
checker implemented in C++. It doesn't use any recent features other
than templates so It is quite portable, the only header included
is `<cassert>` to panic when the borrow rules are not met.

## Writeup

I wanted to implement a borrow checker in C, It seemed like a cool
project for an afternoon. A borrow checker (at least, my borrow checker)
is a system that imposes the following rules:
- There can be infinite (many) immutable references to a value
- There can be only one mutable referene to a value
- The two rules above cannot happen togheter
- The lifetime of a value is boud to the owner's scope (so references
  must be invalidated if the owner goes out of scope)
  
If any of the aforementioned rules does not hold, the code panics (an
assertion fails). Languages like Rust have this incorporated in the
compiler, but for all the other folks this needs to be implemented
manually. While creating a compile-time borrow checker is not trivial
at all, the C++ committee is pushing more and more safety features into
the language. A [famous proposal](https://safecpp.org/draft.html#introduction)
for a full-fledged C++ borrow cheker was
made but the context was quite... controversial. From my understanding,
some safety features were already discussed and "almost" ready to be
implemented, we know that the C++ standard moves very slow and features
take years or decades to be implemented. This proposal came out of nowere
with a full implementation (lots of hours of work) but was "never"
properly discussed with the community, so the future of this proposal is
quite uncertain.

Well, enought talking, let's get into business.
I am not building one of that (but maybe I will eventually), right
now I just want to create a run-time borrow checker that panics when
the rules are not met. While this is not as powerful as a compile time
one, It follows the principle of "fail early": if we made some "unsafe"
variable declarations we should be punished as soon as possible when we
reach that code.

Now. I tried to write everything in C, trust me, but It wasn't quite..
let's say, _ergonomic_. C does not have the concept of constructors
and destructors bound to the scope of a variable, and I needed them
since I need to know when a reference to a variable goes out of scope,
among many other things. I tried creating a sort of constructor-desctructor
with macros but It was _very_ ugly. Something like this:
```cpp
#define CHECK(NAME, ...) { \
  constructor_ ##Check(& NAME); \
  __VA_ARGS__ \
  dtor_ ##Check(& NAME); \
} (void)0

...

int x, y, z;
CHECK(x,
  CHECK(y,
    CHECK(z,
	  z = x + y;
    );
  );
);
```

Cool, not practical. So I moved to C++ and everything was much easier.
The owner of a value wraps the value in an inner context, and the
references hold a pointer to this context. It is very similar to
smart pointer: the context contains a counter for mutable and
immutable references that gets incremented when a new reference gets
created and removed when a reference goes out of scope. If the owner's
destructor gets called and the counters are not 0, that means that
the references are not valid anymore so It panics. In particular, I
created the following calsses:
- `Val<T>`: An immutable value of type `T`
- `ValMut<T>`: A mutable value of type `T`
- `ValRef<T>`: An immutable reference to a value (that It can be both
  mutable or immutable) of type `T`
- `ValMutRef<T>`: A mutable reference to a mutable value of type `T`

You use `getRef()` or `getRefMut()` to get the reference types.

For example, you can have multiple immutable references of a mutable
value:

```cpp
check::Val<int> x = 1;
check::ValRef<int> a = x.getRef();   // OK
check::ValRef<int> a2 = x.getRef();  // OK
assert(a.get() == 1);
assert(a2.get() == 1);
```

But you cannot have multiple mutable references or both mutable and
immutable references:

```cpp
check::ValRef<int> b = y.getRef();
check::ValRef<int> b2 = y.getRef();
assert(b.get() == 3);
assert(b2.get() == 3);
//check::ValMutRef<int> b3 = y.getMutRef(); // FAIL!

```

When you create a mutable reference, you cannot use the owner to access
the value until all the references to that owner go out of scope:

```cpp
check::ValMut<int> c = 10;
check::ValMutRef<int> d = c.getMutRef();
// assert(c.get() == 10); // FAIL!
assert(d.get() == 10);
```

Overall I had great fun with this little project, I will probably add
this to my own [standard library](https://github.com/San7o/tenno-tl)
so I can reuse It easily.

Its already time for the next project lol. Bye.

Note: I did not use any form of AI neither I looked at other's work.
