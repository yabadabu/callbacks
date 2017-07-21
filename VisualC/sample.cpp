#include <map>
#include <cstdio>
#include "callbacks.h"

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


// -------------------------------------------------
void test()
{
  typedef jaba::function_ref<void(int)> TCallback;
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

  TCallback c1 = [&b](int x) { b.method1(x); };
  c1(6);
  TCallback c2 = c1;
  c2(7);

  // ------------------------------------------
  TMsgBus<TCallback> bus;
    bus.add(10, [&b](int x) { b.method1(x); });
    bus.add(10, [&b](int x) { b.method2(x); });
    bus.add(10, [&d1](int x) { d1.method1(x); });
    bus.add(10, lambda1);
    bus.add(10, publicFn);
    bus.add(10, TCallback::make< CBase, &CBase::method1 >(&b));
    //bus.add(10, 3);     // Now fails with the error: 'term does not evaluate to a function taking 1 argument inside the callacks.h
  bus.on(10);
}


// ------------------------------------------------------
int main()
{
  test();
  return 0;
}

