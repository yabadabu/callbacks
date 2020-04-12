#pragma once

#include <algorithm>
#include <vector>
#include "callbacks.h"

namespace jaba {

  namespace bus {

    namespace internal {

      // ------------------------------------------
      template< typename TMsg >
      class TMsgBus {

        typedef Callback<void(TMsg&), 24> TCB;

        struct TSlot {
          TCB cb;
          int priority = 0;
          bool operator<(const TSlot& other) const {
            return priority < other.priority;
          }
        };

      public:

        TMsgBus() {
          printf("Creating bus with slots of size %lld - %lld\n", sizeof(TSlot), sizeof(TCB));
        }

        void add(TCB cb, int priority) {
          TSlot s;
          s.cb = cb;
          s.priority = priority;
          auto it = std::lower_bound(slots.begin(), slots.end(), s);
          slots.insert(it, s);
        }

        void del(TCB cb) {
          auto it = std::remove_if(
            slots.begin(),
            slots.end(),
            [cb](const TSlot& s) {
              return s.cb == cb;
            });

          slots.erase(it, slots.end());
        }

        void on(TMsg& m) {
          for (auto s : slots)
            s.cb(m);
        }

      private:
        std::vector< TSlot > slots;
      };

      template< typename TMsg >
      TMsgBus<TMsg>& getBus() {
        static TMsgBus<TMsg> bus;
        return bus;
      }

    }

    template< typename T, typename M >
    void subscribe(T* obj, void (T::* method)(M&), int priority = 0) {
      internal::getBus< M >().add([obj, method](M& msg) {
        (obj->*method)(msg);
        }, priority);
    }

    template< typename T, typename M >
    void unsubscribe(T* obj, void (T::* method)(M&)) {
      internal::getBus< M >().del([obj, method](M& msg) {
        (obj->*method)(msg);
        });
    }

    template< typename M >
    void emitRef(M& m) {
      internal::getBus<M>().on(m);
    }

    template< typename M >
    void emit(M m) {
      internal::getBus<M>().on(m);
    }


  }

}


