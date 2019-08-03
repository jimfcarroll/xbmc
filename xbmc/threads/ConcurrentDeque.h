/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <deque>
#include <atomic>
#include "threads/Condition.h"

#define CONCURRENT_DEQUE_FAST_SPIN_ITERS 1000000


namespace XbmcThreads
{

  class bad_optional_access : std::exception {
  };

  template<typename T> class optional {
  private:
    T m_value;
    bool m_set;

  public:
    inline optional() : m_set(false) {}
    inline optional(T&& value) : m_value(value), m_set(true) {}

    inline optional(optional<T>&& other) : m_value(std::move(other.m_value)), m_set(other.m_set) {}

    inline bool has_value() { return m_set; }
    inline operator bool() { return m_set; }

    inline T value() throw(bad_optional_access) {
      if (m_set)
        return std::move(m_value);
      throw bad_optional_access();
    }
  };

  template<typename T> class ConcurrentDeque
  {
  private:

    template<typename X> class HasValues {
      std::deque<X>* m_q;
    public:
      inline HasValues(std::deque<X>* q) : m_q(q) {}

      inline bool operator!() {
        return m_q->size() == 0;
      }
    };

    ConditionVariable m_intern_cond;
    CCriticalSection mutex;
    std::deque<T> queue;
    HasValues<T> m_predicate;
    TightConditionVariable<HasValues<T> > m_cond;

  public:
    inline ConcurrentDeque() : m_predicate(&queue), m_cond(m_intern_cond, m_predicate) {}
    virtual inline ~ConcurrentDeque() = default;

    // TODO: This doesn't wake subscribers from take(). Need to fix
    inline void notifyAll() {
      CSingleLock l(mutex);
      m_cond.notifyAll();
    }

    inline optional<T> poll() {
      CSingleLock l(mutex);
      if (queue.size() > 0) {
        T ret = queue.front();
        queue.pop_front();

        m_cond.notify();
        return std::move(optional<T>(std::move(ret)));
      }
      return std::move(optional<T>());
    }

    // TODO: need to make ConcurrentDeque::take interruptible
    inline T take() {
      while (true) {
        CSingleLock l(mutex); // lock

        m_cond.wait(mutex); // wait for the state

        if (queue.size() > 0) {
          T ret = queue.front();
          queue.pop_front();

          m_cond.notify();
          return ret;
        }
      }
    }

    inline void put(T val) {
      CSingleLock l(mutex);
      queue.push_back(val);
      m_cond.notify();
    }

    inline int size() {
      CSingleLock l(mutex);
      return queue.size();
    }
  };

}

