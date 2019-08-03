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

#include <functional>
#include <future>
#include <thread>
#include <memory>
#include <atomic>

#include "threads/ConcurrentDeque.h"
#include "threads/Thread.h"
#include "commons/Exception.h"
#include <iostream>

namespace XbmcThreads
{
  namespace internal {
#include "threads/internal/InternalExecutorService.h"
  }

  XBMCCOMMONS_STANDARD_EXCEPTION(ExecutorShutdownExecption);

  /**
   * This is the base class for all ExecutorServices. It is the core management
   * and submit method abstraction.
   */
  class Executor
  {

  protected:
    std::atomic_bool m_isRunning;

    inline Executor() : m_isRunning(true) {}
    virtual ~Executor() {
      std::cout << "Executor dying" << std::endl;
    }

    virtual void Submit(internal::Runnable* runnable) throw (ExecutorShutdownExecption) = 0;

    inline static void Submit(Executor* executor, internal::Runnable* runnable) throw (ExecutorShutdownExecption) {
      return executor->Submit(runnable);
    }

    inline void checkRunningThrow() throw (ExecutorShutdownExecption) {
      if (IsShutdown())
        throw ExecutorShutdownExecption("task submitted to a stopped executor");
    }

  public:
    /**
     * The will perform a graceful shutdown of the Executor. The Executor will subsequently
     * report 'true' when IsShutdown is called even if there are more tasks being processed.
     * Once all of the tasks are processed the IsTerminated will return 'true'
     */
    virtual inline void Shutdown()
    {
      m_isRunning = false;
    }

    virtual inline bool IsShutdown()
    {
      return !m_isRunning;
    }

    virtual bool IsTerminated() = 0;
  };

  /**
   * This is a template wrapper for the given Executor implementations that
   * adds ExecutorService functionality on top of the backing Executor
   * implementation. To use any of the Executor implementations you need
   * to instantiate this template using the selected implementation. For
   * example:
   *
   * ExecutorService<SimpleSyncronousExecutor> simpleSyncExec;
   * simpleSyncExec.submit([]() { cout << "Hello World" << endl; });
   * simpleSyncExec.shutdown();
   *
   * The Executor functionality makes a distinction between a Runnable and
   * a Callable. 'Executing' a Runnable doesn't return a future,  while
   * 'submitting' a Callable does. You should 'execute' a runnable but 'submit'
   * a callable.
   *
   * Note: while it's possible to 'execute' a callable (a function that returns
   * a value) and therefore ignore the return, it's NOT possible to 'submit'
   * a runnable. Attempting to do so will result in a compile error.
   */
  template<class P, typename... CArgs> class ExecutorService: public P
  {
  public:
    ExecutorService(CArgs... args) : P(args...) {}
    virtual ~ExecutorService() = default;

    template<class Fn, class... Args> void Execute(Fn&& fn, Args&&... args) throw (ExecutorShutdownExecption)
    {
      std::shared_ptr<internal::Runnable> rr = internal::makeRunnableImplRef(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
      auto ptr = rr.get();
      ptr->this_ref = std::move(rr);
      P::Submit(ptr);
    }

    template<class Fn, class... Args> std::future<typename std::result_of<Fn(Args...)>::type> Submit(Fn&& fn, Args&&... args) throw (ExecutorShutdownExecption)
    {
      typedef typename std::result_of<Fn(Args...)>::type result_type;
      auto rr = internal::AssignPromise<result_type>::assignPromise(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
      std::future<result_type> future = rr.get()->future();
      auto ptr = rr.get();
      ptr->this_ref = std::move(rr);
      P::Submit(ptr);
      return future;
    }
  };

  /**
   * This class is an implementation of an Executor that simply runs the
   * submitted functions synchronously in the thread that did the submission.
   */
  class SimpleSyncronousExecutor: public Executor
  {

    inline void junkFunc(int i) {}

  public:
    inline SimpleSyncronousExecutor() = default;
    virtual inline ~SimpleSyncronousExecutor() = default;

    virtual bool IsTerminated() {
      return IsShutdown();
    }

  protected:
    virtual inline void Submit(internal::Runnable* runnable) throw (ExecutorShutdownExecption)
    {
      checkRunningThrow();
      std::shared_ptr<internal::Runnable> localRr;
      localRr.swap(runnable->this_ref);
      runnable->Run();
    }

  };

  /**
   * This class is an implementation of an Executor that simply runs the
   * submitted function asynchronously in a detached thread dynamically
   * created upon submission of the task.
   */
  class DynamicThreadExecutor: public Executor
  {
  public:
    DynamicThreadExecutor() = default;
    virtual ~DynamicThreadExecutor() = default;

    /**
     * This isn't exactly right but the threads are detached.
     */
    virtual bool IsTerminated() {
      return IsShutdown();
    }

  protected:
    virtual void Submit(internal::Runnable* runnable)  throw (ExecutorShutdownExecption);
  };

  /**
   * This class is an implementation of an Executor that runs queues the
   * submitted functions to a fixed size thread pool of worker threads.
   */
  class ThreadPoolExecutor: public Executor
  {
  private:
    ConcurrentDeque<internal::Runnable*> m_queue;
    std::thread** threads;
    int numThreads;

    static void threadPoolExecutorServiceRun(ThreadPoolExecutor*);

  public:
    ThreadPoolExecutor(int numThreads);
    virtual ~ThreadPoolExecutor();

    virtual void Shutdown();

    virtual inline bool IsTerminated()
    {
      if (!IsShutdown())
        return false;
      return m_queue.size() == 0;
    }

    // I really hate this but until there's a proper dependency injection framework we
    // don't have much of a choice here.
    static std::shared_ptr<ExecutorService<ThreadPoolExecutor, int> > GetMainExecutor();

  protected:

    virtual inline void Submit(internal::Runnable* runnable)  throw (ExecutorShutdownExecption)
    {
      checkRunningThrow();
      m_queue.put(runnable);
    }
  };

  typedef ExecutorService<ThreadPoolExecutor, int> ThreadPoolExecutorService;

  /**
   * This is a special executor allows for ordering guarantees. It will sequentially
   * feed the underlying executor the tasks thereby making sure that their executions
   * don't overlap and they are completed in the order they are submitted.
   * It can be used as follows:
   *
   * ExecutorService<ThreadPoolExecutor, int> executor(10); // an executor with 10 threads.
   * ...
   * ExecutorService<SequentialFeedExecutor, Executor*> jobQueue(&executor);
   *
   * If the underlying executor is ONLY going to be used in a SequentialFeedExecutor then
   * a better approach would be to use a single threaded ThreadPoolExecutor:
   *
   * ExecutorService<ThreadPoolExecutor, int> jobQueue(1);
   *
   * Since there is only 1 worker thread in this case the same guarantees apply but you're not
   * wasting multiple idle threads.
   */
  class SequentialFeedExecutor: public Executor, public internal::Runnable {
  private:
    bool m_currentlyPending;
    Executor* underlying;
    std::deque<internal::Runnable*> m_queue;
    CCriticalSection mutex;

  public:
    inline SequentialFeedExecutor(Executor* executor) : m_currentlyPending(false), underlying(executor) {}

    virtual void Submit(internal::Runnable* runnable) throw (ExecutorShutdownExecption);

    virtual void Run();

    virtual inline bool IsShutdown()
    {
      return !m_isRunning || underlying->IsShutdown();
    }

    virtual bool IsTerminated() {
      return IsShutdown() && !m_currentlyPending;
    }
  };

  typedef ExecutorService<SequentialFeedExecutor, Executor*> SequentialFeedExecutorService;
}
