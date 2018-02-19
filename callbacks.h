#ifndef INC_JABA_CALLBACKS_H_
#define INC_JABA_CALLBACKS_H_

#include <cassert>
#include <type_traits>

#include <new>

namespace jaba {

  // ------------------------------------------------------------------
  template <typename TFn, size_t N = 24>
  class Delegate;

  // ------------------------------------------------------------------
  template <typename TResult, typename ...Args, size_t N>
  class Delegate<TResult(Args...), N> {

    // Pointer to the function which will do the real code
    TResult(*caller)(const void*, Args...) = nullptr;

    // To store the Func lambda args
    char     storage[N];

    // Function template generator that will restore the erased type to the original calling function
    // from the fn_voids
    template< typename Func >
    static TResult callGenerator(const void* fn_void, Args... args) {
      return (*reinterpret_cast<const Func *>(fn_void))(std::forward<Args...>(args...));
    }

  public:

    // Single ctor from a callable
    template< typename Func >
    Delegate(Func f)
      : caller(&callGenerator<Func>) {

      // Too may lambda capture args might require more space in the local storage
      static_assert(sizeof(Func) <= N, "Need more space to store callback. Increase the template arg N of the callback.");

      // Copy f into storage erasing the type
      new (storage) Func(static_cast<Func&&>(f));

      // printf("Using %zd/%zd bytes of the storage\n", sizeof(Func), N);
    }

    // Operator() forwards call to our specific callGenerator with the given args
    TResult operator()(Args... args) const {
      return caller(storage, std::forward<Args...>(args...));
    }

  };
}

#endif
