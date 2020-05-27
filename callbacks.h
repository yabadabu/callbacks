#ifndef INC_JABA_CALLBACKS_H_
#define INC_JABA_CALLBACKS_H_

#include <cassert>
#include <new>

namespace jaba {

  // ------------------------------------------------------------------
  // N could be as small as 8 if you just need an arg in the lambda's
  template <typename Fn, size_t N = 24>
  class Callback;

  // ------------------------------------------------------------------
  template <typename Result, typename ...Args, size_t N>
  class Callback<Result(Args...), N> {

    // Pointer to the function which will do the real code
    Result(*caller)(const void*, Args...) = nullptr;

    // To store the Func lambda args
    char     storage[N] = { 0 };

    // Function template generator that will restore the erased type to the original calling function
    // from the fn_voids
    template< typename Func >
    static Result callGenerator(const void* fn_void, Args... args) {
      return (*reinterpret_cast<const Func*>(fn_void))(std::forward<Args>(args)...);
    }

  public:

    Callback() { }

    // Single ctor from a callable
    template< typename Func >
    Callback(Func f)
      : caller(&callGenerator<Func>) {

      // Too may lambda capture args might require more space in the local storage
      static_assert(sizeof(Func) <= N, "Need more space to store callback. Increase the template arg N of the callback.");

      // Copy f into storage erasing the type
      new (storage) Func(static_cast<Func&&>(f));

      // printf("Using %zd/%zd bytes of the storage\n", sizeof(Func), N);
    }

    // Operator() forwards call to our specific callGenerator with the given args
    Result operator()(Args... args) const {
      assert(caller);
      return caller(storage, std::forward<Args>(args)...);
    }

    bool operator==(const Callback& other) const {
      return memcmp(storage, other.storage, N) == 0;
    }

    operator bool() const { return caller != nullptr; }

  };
}

#endif
