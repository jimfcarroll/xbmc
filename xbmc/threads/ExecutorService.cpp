/*
 * Executor.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jim
 */

#include "ExecutorService.h"
#include "utils/GlobalsHandling.h"

#define ITERS 1000000

namespace XbmcThreads
{

  static void dynamicThreadFunc(internal::Runnable* runnable) {
    std::shared_ptr<internal::Runnable> localRr;
    localRr.swap(runnable->this_ref);
    runnable->Run();
  }

  void DynamicThreadExecutor::Submit(internal::Runnable* runnable) throw (ExecutorShutdownExecption)
  {
    checkRunningThrow();
    std::thread th(dynamicThreadFunc, runnable);
    th.detach();
  }

  static int calculateNumCores() {
    // TODO: do this for real
    return 10;
  }

  class ThreadPoolExecutorDoh : public ThreadPoolExecutorService {
  public:
    ThreadPoolExecutorDoh() : ThreadPoolExecutorService(calculateNumCores()) {}
  };

  XBMC_GLOBAL_REF(ThreadPoolExecutorDoh, g_ThreadPoolExecutor);

  std::shared_ptr<ThreadPoolExecutorService> ThreadPoolExecutor::GetMainExecutor() {
    return std::shared_ptr<ThreadPoolExecutorService>(xbmcutil::GlobalsSingleton<ThreadPoolExecutorDoh>::getInstance());
  }

  void ThreadPoolExecutor::threadPoolExecutorServiceRun(ThreadPoolExecutor* ths)
  {
    while (ths -> m_isRunning) {
      try
      {
        // TODO: need to make ConcurrentDeque::take interruptible
        optional<internal::Runnable*> result = ths->m_queue.poll();
        internal::Runnable* runnable = result.has_value() ? result.value() : nullptr;
        if (runnable) {
          std::shared_ptr<internal::Runnable> localRr;
          localRr.swap(runnable->this_ref); // clears the self reference.
          runnable->Run();
        } // localRr out of scope.
        else
        {
          std::this_thread::yield();
        }
      }
      catch (std::exception& e)
      {
        // TODO: add log
      }
      catch (...)
      {
        // TODO: add log
      }
    }
  }

  ThreadPoolExecutor::ThreadPoolExecutor(int i) : numThreads(i)
  {
    threads = new std::thread*[i];
    for (int i = 0; i < numThreads; i++) {
      threads[i] = new std::thread(threadPoolExecutorServiceRun, this);
    }
  }

  void ThreadPoolExecutor::Shutdown()
  {
    if (!IsShutdown()) {
      m_isRunning = false;
      m_queue.notifyAll();
      for (int i = 0; i < numThreads; i++) {
        std::thread* th = threads[i];
        threads[i] = nullptr;
        if (th) {
          th->join();
          delete th;
        }
      }

      delete [] threads;
      threads = nullptr;
    }
  }

  ThreadPoolExecutor::~ThreadPoolExecutor()
  {
    Shutdown();
  }

  void SequentialFeedExecutor::Submit(internal::Runnable* runnable) throw (ExecutorShutdownExecption)
  {
    checkRunningThrow();
    CSingleLock l(mutex);
    m_queue.push_back(runnable);
    if (!m_currentlyPending) {
      m_currentlyPending = true;
      Executor::Submit(underlying, this);
    }
  }

  void SequentialFeedExecutor::Run()
  {
    CSingleLock l(mutex);
    while (m_queue.size() > 0) {
      auto runnable = m_queue.front();
      m_queue.pop_front();

      try {
        CSingleExit ex(mutex);
        runnable->Run();
      }
      catch (...)
      {
        if (m_queue.size() > 0) {
          m_currentlyPending = true;
          Executor::Submit(underlying, this);
        }
        else
          m_currentlyPending = false;
        throw;
      }
    }

    m_currentlyPending = false; // nothing left and we're leaving the Run method
  }
}
