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

#pragma once

#include <cassert>

#ifdef CHECK_DEBUG

// use CHECK_TRAP() to set a gdb breakpoint
#ifdef __linux__
#   if defined(__GNUC__) && (defined(__i386) || defined(__x86_64))
#       define CHECK_TRAP() asm volatile ("int $3") // NOLINT
#   else // Fall back to the generic way.
#       include <signal.h>
#       define CHECK_TRAP() raise(SIGTRAP)
#    endif
#elif defined(_MSC_VER)  // windows
#    define CHECK_TRAP() __debugbreak()
#elif defined(__MINGW32__)
    extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#    define CHECK_TRAP() DebugBreak()
#endif

#endif  // CHECK_DEBUG

namespace check
{

////////////////////////////////////////////////
// Types
////////////////////////////////////////////////
  
template<typename T>
class ValContext;
template<typename T>
class Val;
template<typename T>
class ValMut;
template<typename T>
class ValRef;
template<typename T>
class ValMutRef;

template<typename T>
class ValContext
{
public:
  // constructors
  ValContext() : refs(0), mutRefs(0), val(T()), moved(false) {};
  ValContext(T in) : refs(0), mutRefs(0), val(in), moved(false) {};
  ValContext(ValContext &&other);
  // move constructor

  //operators
  ValContext<T>& operator=(const ValContext<T>& other); 
  ValContext<T>& operator=(ValContext<T>&& other); // move

  // methods
  void addRef();
  void addMutRef();

  void removeRef();
  void removeMutRef();

  unsigned int getRefs();
  unsigned int getMutRefs();

  T getVal();
  void setVal(T val);

private:
  unsigned int refs;
  unsigned int mutRefs;
  T val;
  bool moved;
};

template<typename T>
class Val
// immutable value
{
public:
  friend class ValMut<T>;
  friend class ValRef<T>;

  // constructors
  Val();
  Val(T in);
  Val(Val &other);
  // copy constructor
  Val(Val &&other);
  // move constructor

  ~Val();
  // destructor

  // methods
  T get();
  ValRef<T> getRef();

private:
  ValContext<T> context;
};

template<typename T>
class ValMut
{
public:
  friend class ValRef<T>;
  friend class ValMutRef<T>;

  // constructors
  ValMut();
  ValMut(T in);
  ValMut(Val<T> from);
  ValMut(ValMut<T> &other);
  // copy constructor
  ValMut(ValMut<T> &&other);
  // move constructor

  ~ValMut();
  // destructor

  // methods
  T get();
  void set(T newVal);
  ValRef<T> getRef();
  ValMutRef<T> getMutRef();

private:
  ValContext<T> context;
};

template<typename T>
class ValRef
{
public:
  friend class ValMutRef<T>;

  // constructors
  ValRef() = delete;
  ValRef(Val<T> *from);
  ValRef(ValMut<T> *from);
  // todo copy constructor

  ~ValRef();
  // destructor

  // methods
  T get();

private:
  ValContext<T> *context;
};

template<typename T>
class ValMutRef
{
public:
  // constructors
  ValMutRef() = delete;
  ValMutRef(ValMut<T> *from);

  ~ValMutRef();
  // destructor

  // operators
  ValMutRef<T>& operator=(ValMutRef<T>& other) = delete;
  ValMutRef<T>& operator=(ValMutRef<T>&& other); // move

  // methods
  T get();
  void set(T in);

private:
  ValContext<T> *context;
};

////////////////////////////////////////////////
// Implementations
////////////////////////////////////////////////

// ValContext implementations

template<typename T>
ValContext<T>::ValContext(ValContext &&other)
{
  *this = other;
  other.moved = true;
  return;
}

template<typename T>
void ValContext<T>::addRef()
{
  assert(!this->moved && "moved");
  this->refs++;
  return;
}

template<typename T>
ValContext<T>& ValContext<T>::operator=(const ValContext<T>& other)
{
  this->moved = other.moved;
  this->refs = other.refs;
  this->mutRefs = other.mutRefs;
  this->val = other.val;
  return *this;
}

template<typename T>
ValContext<T>& ValContext<T>::operator=(ValContext<T>&& other)
{
  *this = other;
  other.moved = true;
  return *this;
}

template<typename T>
void ValContext<T>::addMutRef()
{
  assert(!this->moved && "moved");
  this->mutRefs++;
  return;
}

template<typename T>
void ValContext<T>::removeRef()
{
  assert(!this->moved && "moved");
  assert(this->refs > 0 &&
	 "A reference was removed but there were no references");
  this->refs--;
  return;
}

template<typename T>
void ValContext<T>::removeMutRef()
{
  assert(!this->moved && "moved");
  assert(this->mutRefs > 0 &&
	 "A mut reference was removed but there were no mut references");
  this->mutRefs--;
  return;
}

template<typename T>
unsigned int ValContext<T>::getRefs()
{
  assert(!this->moved && "moved");
  return this->refs;
}

template<typename T>
unsigned int ValContext<T>::getMutRefs()
{
  assert(!this->moved && "moved");
  return this->mutRefs;
}

template<typename T>
T ValContext<T>::getVal()
{
  assert(!this->moved && "moved");
  return this->val;
}

template<typename T>
void ValContext<T>::setVal(T val)
{
  assert(!this->moved && "moved");
  this->val = val;
  return;
}

// Val implementation
  
template<typename T>
Val<T>::Val()
{
  this->context = ValContext<T>();
  return;
}

template<typename T>
Val<T>::Val(T in)
{
  this->context = ValContext<T>(in);
  return;
}

template<typename T>
Val<T>::Val(Val &other)
{
  this->context = ValContext(other.context.getVal());
  return;
}

template<typename T>
Val<T>::Val(Val &&other)
{
  this->context = (ValContext<T>&&) other.context; // move
  return;
}

template<typename T>
Val<T>::~Val()
{
  assert(this->context.getRefs() == 0 &&
	 "ValRefs lifetime excedds owner's lifetime");
  return;
}

template<typename T>
T Val<T>::get()
{
  assert(this->context.getRefs() == 0 &&
	 "ValRefs lifetime exceeds owner's lifetime");
  return this->context.getVal();
}

template<typename T>
ValRef<T> Val<T>::getRef()
{
  return ValRef<T>(this);
}

// ValMut implementations

template<typename T>
ValMut<T>::ValMut()
{
  this->context = ValContext<T>();
  return;
}

template<typename T>
ValMut<T>::ValMut(T in)
{
  this->context = ValContext<T>(in);
  return;
}

template<typename T>
ValMut<T>::ValMut(Val<T> in)
{
  this->context = in.context;
  return;
}

template<typename T>
ValMut<T>::ValMut(ValMut<T> &other)
{
  this->context = ValContext(other.context.getVal());
  return;
}

template<typename T>
ValMut<T>::ValMut(ValMut &&other)
{
  this->context = (ValContext<T>&&) other.context;
  return;
}
 
template<typename T>
ValMut<T>::~ValMut()
{
  assert(this->context.getRefs() == 0 &&
	 "ValRefs lifetime excedds owner's lifetime");
  return;
}

template<typename T>
T ValMut<T>::get()
{
  assert(this->context.getMutRefs() == 0 &&
	 "Cannot be used until all the mut refs go out of scope");
  return this->context.getVal();
}
  
template<typename T>
void ValMut<T>::set(T newVal)
{
  this->context.setVal(newVal);
  return;
}

template<typename T>
ValRef<T> ValMut<T>::getRef()
{
  return ValRef<T>(this);
}

template<typename T>
ValMutRef<T> ValMut<T>::getMutRef()
{
  return ValMutRef<T>(this);
}

// ValRef implementations

template<typename T>
ValRef<T>::ValRef(Val<T> *from)
{
  this->context = &from->context;
  this->context->addRef();
  return;
}

template<typename T>
ValRef<T>::ValRef(ValMut<T> *from)
{
  this->context = &from->context;
  this->context->addRef();
  return;
}

template<typename T>
ValRef<T>::~ValRef()
{
  assert(this->context != nullptr && "ValRef not initialized");
  this->context->removeRef();
}

template<typename T>
T ValRef<T>::get()
{
  assert(this->context != nullptr && "ValRef not initialized");
  return this->context->getVal();
}

// ValMutRef implementations
  
template<typename T>
ValMutRef<T>::ValMutRef(ValMut<T> *from)
{
  assert(from->context.getRefs() == 0 &&
	 "Tried to create a mut refs when immut ones exist");
  assert(from->context.getMutRefs() == 0 &&
	 "Tried to create multiple mut refs to the same object");
  this->context = &from->context;
  this->context->addMutRef();
}

template<typename T>
ValMutRef<T>& ValMutRef<T>::operator=(ValMutRef<T>&& other)
{
  assert(other->context.getRefs() == 0 &&
	 "Tried to create a mut refs when immut ones exist");
  this->context = other->context;
  other->context = nullptr;
  return *this;
}

template<typename T>
ValMutRef<T>::~ValMutRef()
{
  assert(this->context != nullptr && "moved");
  this->context->removeMutRef();
}

 
template<typename T>
T ValMutRef<T>::get()
{
  assert(this->context != nullptr && "moved");
  assert(this->context->getRefs() == 0 &&
	 "Mut and Immut refs of the same object detected");
  return this->context->getVal();
}

template<typename T>
void ValMutRef<T>::set(T in)
{
  assert(this->context != nullptr && "moved");
  assert(this->context->getRefs() == 0 &&
	 "Mut and Immut refs of the same object detected");
  this->context->setVal(in);
  return;
}
  
} // namespace check
