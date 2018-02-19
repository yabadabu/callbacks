#include <map>
#include <cstdio>
#include "callbacks.h"
#include "fast_func.h"

// ------------------------------------------
template< typename TCallbackType >
class TMsgBus {
  std::multimap< unsigned, TCallbackType > callbacks;
public:
  void add(unsigned id, TCallbackType cb) {
    callbacks.insert(std::pair<unsigned, TCallbackType >(id, cb));
  }
  void on(unsigned id) {
    auto range = callbacks.equal_range(id);
    auto it = range.first;
    while (it != range.second) {
      it->second(id);
      ++it;
    }
  }
};

// -------------------------------------------------
class CBase {
  int id = 7;
public:
  virtual void method1(int a) {
    printf("Hi from CBase::method1(%d)\n", a);
  }
  void method2(int a) {
    printf("Hi from CBase::method2(%d)\n", a);
  }
  void void_method() {
    printf("Hi from CBase::void_method\n");
  }
};

class CDerived1 : public CBase {
public:
  void method1(int a) override {
    printf("Hi from CDerived1::method1(%d)\n", a);
  }
};

class CDummy {

};

class CDerived2 : public CDummy, public CBase {
public:
  void method1(int a) override {
    printf("Hi from CDerived2::method1(%d)\n", a);
  }
};

void publicFn(int a) {
  printf("Hi from publicFn(%d)\n", a);
}


#include <cassert>
#include <memory>

// struct Lambda declaration
template<typename Func> struct Lambda;

// Implementation details cannot be instantiated in user code
namespace details
{
  // Lambda_impl is an internal class that adds the ability to execute to Lambdas.
  template<typename Func> struct Lambda_impl;

  template <typename Out, typename... In>
  struct Lambda_impl<Out(In...)>
  {
    friend struct Lambda<Out(In...)>;

    ~Lambda_impl() { if (deleteLambda != nullptr) deleteLambda(func_ptr); }

  private:

    template<typename Func>
    Lambda_impl(Func&& func)
      : func_ptr(new Func(std::move(func)))
      , deleteLambda([](void *func_ptr) { delete (Func *)func_ptr; })
      , executeLambda([](void *func, In... arguments) -> Out
    {
      return ((Func *)func)->operator()(arguments...);
    }) { }

    void *func_ptr;
    Out(*executeLambda)(void *, In...);
    void(*deleteLambda)(void *);
  };

  // Template specialization for function returning void
  template <typename... In> struct Lambda_impl<void(In...)>
  {
    friend struct Lambda<void(In...)>;

    ~Lambda_impl() { if (deleteLambda != nullptr) deleteLambda(func_ptr); }

  private:

    template<typename Func>
    Lambda_impl(Func&& func)
      : func_ptr(new Func(std::move(func)))
      , deleteLambda([](void *func_ptr) { delete (Func *)func_ptr; })
      , executeLambda([](void *func, In... arguments)
    {
      return ((Func *)func)->operator()(arguments...);
    }) { }

    void *func_ptr;
    void(*executeLambda)(void *, In...);
    void(*deleteLambda)(void *);
  };
} // end of details namespace

  // Lambda implementation can be instantiated directly in user code.
template <typename Out, typename ...In> struct Lambda<Out(In...)>
{
  // Nullary constructor
  Lambda() { }

  // Unary constructor
  template<typename Func>
  Lambda(Func&& func)
    : impl(new details::Lambda_impl<Out(In...)>(std::forward<Func>(func))) { }

  // Destructor
  virtual ~Lambda() { }

  // Move constructor
  Lambda(Lambda<Out(In...)>&& other) { impl.swap(other.impl); }

  // Boolean operator overload
  operator bool() { return impl != nullptr; }

  // Lambda call operator overload
  Out operator()(In ... in)
  {
    assert(impl != nullptr);
    impl->executeLambda(impl->func_ptr, in...);
  }

private:
  // No copy constructor
  Lambda(const Lambda& other);

  std::unique_ptr<details::Lambda_impl<Out(In...)>> impl;
};


int test_lambda()
{
  typedef Lambda<void(int)> TLambda;
  int z = 10;
  TLambda func = [z](int a) { 
    int r = a + z;
    std::printf("Testing TLambda %d, z=%d -> R=%d\n", a, z, r);
  };
  printf("Sizeof TLambda is %zd\n", sizeof(TLambda));

  func(10);
  func(5);

  return 0;
}

#include <new>

#define FUNC_FORWARD(type, value) static_cast<type &&>(value)

template <typename TFn>
struct Delegate;

// ------------------------------------------------------------------
// Lambda implementation can be instantiated directly in user code.
template <typename TResult>
struct Delegate<TResult(int)> {
  TResult(*generator)(void*, int) = nullptr;
  char     storage[24];
  
  void* getStorage() { return &storage[0]; }

  template< typename Func >
  static TResult callGenerator( void* fn_void, int id ) {
    return const_cast<Func &>(*reinterpret_cast<const Func *>(fn_void))(id);
  }

  template< typename Func >
  Delegate(Func f)
  {
    new (getStorage()) Func(FUNC_FORWARD(Func, f));
    generator = &callGenerator<Func>;
  }

  TResult call(int id) {
    generator(getStorage(), id);
  }

};

void static_int(int id) {
  printf("Hi from static_int %d\n", id );
}

int test2()
{
  typedef Delegate<void(int)> TCB;
  TCB c1(static_int);
  printf("sizeof of TCB is %zd\n", sizeof( TCB ));
  float f = 3.14f;
  TCB c2 = [f](int id) {
    printf("Hi from lambda with f = %f. id=%d\n", f, id);
  };
  TCB c3( c1 );
  c1.call(10);
  c2.call(20);
  c3.call(30);

  printf("sizeof of TCB is %zd\n", sizeof(c2));
  return 0;
}


// -------------------------------------------------
void test()
{

  test_lambda();

  typedef jaba::function_ref<void(int)> TCallback;
  
  //typedef ssvu::FastFunc<void(int)> TCallback;
  CBase b;
  CDerived1 d1;
  CBase* d2 = new CDerived2;
  int id = 100;

  printf("Testing TCallback ----------------\n");
  printf("Sizeof TCallback is %zd\n", sizeof(TCallback));

  // ------------------------------------------
  auto lambda1 = [id](int a) {
    printf("Hi from non-captured-params lambda %d, with params %d\n", a, id);
  };


  //TCallback c1 = [&b](int x) { b.method1(x); };
  //c1(6);
  //TCallback c2 = c1;
  //c2(7);

  // ------------------------------------------
  TMsgBus<TCallback> bus;
    //bus.add(10, [&b](int x) { b.method1(x); });
    //bus.add(10, [&b](int x) { b.method2(x); });
    //bus.add(10, [&d1](int x) { d1.method1(x); });
   // bus.add(10, lambda1);
    bus.add(10, publicFn);
    bus.add(10, TCallback::make< CBase, &CBase::method1 >(&b));
    bus.add(10, TCallback::make< CDerived1, &CDerived1::method1 >(&d1));
    bus.add(10, TCallback::make< CBase, &CBase::method1 >(&d1));
    //bus.add(10, 3);     // Now fails with the error: 'term does not evaluate to a function taking 1 argument inside the callacks.h
  bus.on(10);
}


// ------------------------------------------------------
int main()
{
  test2();
  return 0;
}

