#ifndef INC_JABA_CALLBACKS_H_
#define INC_JABA_CALLBACKS_H_

#include <cassert>
#include <type_traits>

// ---------------------------------------------------------
// Based on
// http://foonathan.net/blog/2017/01/20/function-ref-implementation.html
// Added the option to call a member function.
// ---------------------------------------------------------

namespace jaba {

  template <typename Signature>
  class function_ref;

  template <typename Return, typename... Args>
  class function_ref<Return(Args...)>
  {
    //using storage = std::aligned_union<16, void*, Return(*)(Args...)>;
    typedef char storage[sizeof(void*)];
    using callback = Return(*)(void*, Args...);

    storage  storage_;
    callback cb_;

    void* get_memory() noexcept
    {
      return &storage_;
    }
    const void* get_memory() const noexcept
    {
      return &storage_;
    }

  public:
    using signature = Return(Args...);

    function_ref() : cb_(nullptr) {
    }

    // 
    function_ref(Return(*fptr)(Args...))
    {
      using pointer_type = Return(*)(Args...);

      //DEBUG_ASSERT(fptr, detail::precondition_error_handler{},  "function pointer must not be null");
      ::new (get_memory()) pointer_type(fptr);

      cb_ = [](void* memory, Args... args) {
        auto func = *static_cast<pointer_type*>(memory);
        return func(static_cast<Args>(args)...);
      };
    }

    template <typename Functor,
      typename = std::enable_if_t<
          // disable if Functor is a function_ref
          !std::is_same<std::decay_t<Functor>, function_ref>{}
          // disable if Functor not a functor

        >
      >
      function_ref(Functor& f)
      : cb_([](void* memory, Args... args) {
      using ptrt = void*;
      auto  ptr = *static_cast<const ptrt*>(memory);
      auto& func = *static_cast<Functor*>(ptr);
      // deliberately assumes operator(), see further below
      return static_cast<Return>(func(static_cast<Args>(args)...));
    })
    {
      ::new (get_memory()) void*(&f);
    }

    Return operator()(Args... args)
    {
      return cb_(get_memory(), static_cast<Args>(args)...);
    }

    // We will store in ptr the addr of the object who
    // and erase_fn will be the address of a lambda which calls
    // the method of who specified in the template.
    template< class TObj, Return(TObj::*method)(Args...) >
    static function_ref<Return(Args...)> make(TObj* who) {
      function_ref cb;
      ::new (cb.get_memory()) TObj*(who);
      cb.cb_ = [](void* memory, Args... args) -> Return {
        TObj* obj = *reinterpret_cast<TObj**>(memory);
        (obj->*method)(std::forward<Args>(args)...);
      };
      return cb;
    }
  };
}

/*

// Based on
// https://vittorioromeo.info/index/blog/passing_functions_to_functions.html#fn_view_impl
// Crashes on Release

namespace jaba {

template< typename TSignature >
class function_ref;

template <typename TReturn, typename... TArgs>
class function_ref<TReturn(TArgs...)> final
{

using signature_type = TReturn(*)(void*, TArgs...);

void*          ptr;
signature_type stub_fn;

public:

// Send me a fn and we build a lambda to call it
// Disable this method if T is a function_ref, as we
// want to use the copy-ctor
template <typename T, typename = std::enable_if_t<
!std::is_same< std::decay_t<T>, function_ref >{}
// && std::is_callable
>
>
function_ref(T&& x) noexcept : ptr{ (void*)std::addressof(x) }
{
stub_fn = [](void* fn_addr, TArgs... xs) -> TReturn {
printf("Calling at fn_addr %p\n", fn_addr);
return (*reinterpret_cast<std::add_pointer_t<T>>(fn_addr))( std::forward<TArgs>(xs)... );
};
printf("Creating from a fn. stub is %p, fn_addr is %p\n", stub_fn, ptr);
}

// Empty ctor
function_ref() : ptr(nullptr), stub_fn(nullptr) {
}

// ....
~function_ref(){
printf("Dtor...\n");
}

function_ref(const function_ref& other) {
printf("Copy ctor\n");
ptr = other.ptr;
stub_fn = other.stub_fn;
}
function_ref(const function_ref&& other) {
printf("Move ctor\n");
ptr = other.ptr;
stub_fn = other.stub_fn;
}


// Operator () to execute the original fn
decltype(auto) operator()(TArgs... xs) const
noexcept(noexcept(stub_fn(ptr, std::forward<TArgs>(xs)...)))
{
assert(stub_fn);
assert(ptr);
return stub_fn(ptr, std::forward<TArgs>(xs)...);
}

// We will store in ptr the addr of the object who
// and erase_fn will be the address of a lambda which calls
// the method of who specified in the template.
template< class TObj, TReturn(TObj::*method)(TArgs...) >
static function_ref<TReturn(TArgs...)> make(TObj* who) {
function_ref cb;
cb.ptr = who;
cb.stub_fn = [](void* ptr, TArgs... xs) -> TReturn {
TObj* obj = reinterpret_cast<TObj*>(ptr);
(obj->*method)(std::forward<TArgs>(xs)...);
};
return cb;
}
};

}
*/


#endif
