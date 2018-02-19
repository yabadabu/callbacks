# Callback

- A thin callback class (defaults to 32bytes, but can be as short as 16 bytes)
- You don't need to derive from anything to get called, just specify the global function or assign a lambda. 
- No heap allocation ever. The lambda params are saved inside the callback object itself.
- The local storage defaults to 24 bytes, but can be enlarged if required.
- No copy-ctor, no move semantics or destroying the saved lambda.

    // Some type of event registration between uint32_t -> callbacks
    template< typename TCallbackType >
    class TMsgBus {
      std::multimap< uint32_t, TCallbackType > callbacks;
    public:
      void add(uint32_t id, TCallbackType cb) {
        callbacks.insert(std::pair<uint32_t, TCallbackType >(id, cb));
      }
      void on(uint32_t id) {
        auto range = callbacks.equal_range(id);
        auto it = range.first;
        while (it != range.second) {
          it->second(id);
          ++it;
        }
      }
    };

    void test())
    {
      typedef jaba::Callback<void(int)> TCallback;
      CBase b;
      CDerived1 d1;
      CBase* d2 = new CDerived2;
      int id = 100;

      printf("Testing TCallback3 ----------------\n");
      printf("Sizeof TCallback is %zd\n", sizeof(TCallback));

      // ------------------------------------------
      auto lambda1 = [id](int a) {
        printf("Hi from non-captured-params lambda %d, with params %d\n", a, id);
      };

      // ------------------------------------------
      TMsgBus<TCallback> bus;
      {
        bus.add(10, [&b](int x) { b.method1(x); });
        bus.add(10, [&b](int x) { b.method2(x); });
        bus.add(10, [&d1](int x) { d1.method1(x); });
        bus.add(10, lambda1);
        bus.add(10, publicFn);
        bus.add(10, TCallback::make< CBase, &CBase::method1 >(&b));
        bus.add(10, TCallback::make< CBase, &CBase::method1 >(d2));

      }
      bus.on(10);
    }
