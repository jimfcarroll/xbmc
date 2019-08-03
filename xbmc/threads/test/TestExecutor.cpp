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

#include "gtest/gtest.h"
#include "threads/ExecutorService.h"
#include <atomic>

#include "threads/test/TestHelpers.h"

#define TIMEOUT 10000

namespace XbmcThreads
{
  namespace test
  {
    static std::atomic<bool>* abref;
    static int* vref;

    static void sfunc(int i) {
      *vref = i;
      abref->store(true);
    }

    static int sfunc2(int i) {
      return i;
    }

    class Op {
      std::atomic<bool>& execed;
      int& val;

    public:
      Op(std::atomic<bool>& e,int& v) : execed(e), val(v) {}

      Op& operator()(int i) {
        val = i;
        execed.store(true);
        return *this;
      }
    };

    class Op2 {
      int& val;
    public:
      Op2(Op2&& other) : val(other.val) {}
      Op2(int& v) : val(v) {};

      int operator()(int i) {
        val = i;
        return i;
      }
    };

    template<typename ESType> static inline void testRunnable(ESType& executor) {
      std::atomic<bool> execed(false);
      int val = 0;

      abref = &execed;
      vref = &val;

      //==========================================
      // test lambda as rvalue
      executor.Execute([&execed, &val](int i){ val = i; execed.store(true); }, 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================
      // test lambda as lvalue
      auto lambda = [&execed, &val](int i){ val = i; execed.store(true); };
      executor.Execute(lambda, 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================
      // test std::function as lvalue
      std::function<void(int)> f = lambda;
      executor.Execute(f, 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================
      // test w/ straight up function
      executor.Execute(sfunc, 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================
      // test w/ call Operator class
      executor.Execute(Op(execed, val), 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================
      // test w/ call Operator class as lvalue
      Op op(execed, val);
      executor.Execute(op, 1);

      EXPECT_TRUE(waitFor([&execed]() { return bool(execed); }, TIMEOUT));
      EXPECT_EQ(1, val);

      execed.store(false); // reset
      val = 0;
      //==========================================

      executor.Shutdown();
      EXPECT_TRUE(executor.IsShutdown());

      bool caught = false;
      try {
        executor.Execute([](){});
      } catch (ExecutorShutdownExecption &e) {
        caught = true;
      }
      EXPECT_TRUE(caught);

      EXPECT_TRUE(waitFor([&executor]() { return executor.IsTerminated(); }, TIMEOUT));
    }

    TEST(TestExecutor, SimpleRunnableTest)
    {
      ExecutorService<SimpleSyncronousExecutor> executor;
      testRunnable(executor);
      ExecutorService<DynamicThreadExecutor> de;
      testRunnable(de);
      ExecutorService<ThreadPoolExecutor, int> tpes1(1);
      testRunnable(tpes1);
      ExecutorService<ThreadPoolExecutor, int> tpes(10);
      testRunnable(tpes);

      ExecutorService<SimpleSyncronousExecutor> executor2;
      ExecutorService<SequentialFeedExecutor, Executor*> sfe01(&executor2);
      testRunnable(sfe01);

      ExecutorService<DynamicThreadExecutor> de2;
      ExecutorService<SequentialFeedExecutor,Executor*> sfe02(&de2);
      testRunnable(sfe02);


      ExecutorService<ThreadPoolExecutor, int> tpes2(10);
      ExecutorService<SequentialFeedExecutor,Executor*> sfe1(&tpes2);
      testRunnable(sfe1);

      ExecutorService<ThreadPoolExecutor, int> tpes3(1);
      ExecutorService<SequentialFeedExecutor,Executor*> sfe2(&tpes3);
      testRunnable(sfe2);
    }

    template<typename ESType> static inline void testCallable(ESType& executor) {
      int val = 0;

      //==========================================
      // test lambda as rvalue
      std::future<int> fut = executor.Submit([](int i){ return i; }, 1);

      EXPECT_EQ(1, fut.get());

      // test submitting as a Runnable
      executor.Execute([&val](int i){ val = i; return i; }, 1);
      EXPECT_TRUE(waitFor([&val]() { return val == 1; }, TIMEOUT));
      val = 0;
      //==========================================
      // test lambda as lvalue
      auto lambda = [&val](int i){ val = i; return i; };
      fut = executor.Submit(lambda, 1);

      EXPECT_EQ(1, fut.get());

      // test submitting as a Runnable
      val = 0;
      executor.Execute(lambda, 1);
      EXPECT_TRUE(waitFor([&val]() { return val == 1; }, TIMEOUT));
      val = 0;
      //==========================================
      // test std::function as lvalue
      std::function<int(int)> f = lambda;
      fut = executor.Submit(f, 1);

      EXPECT_EQ(1, fut.get());

      // test submitting as a Runnable
      val = 0;
      executor.Execute(f, 1);
      EXPECT_TRUE(waitFor([&val]() { return val == 1; }, TIMEOUT));
      val = 0;
      //==========================================
      // test w/ straight up function
      fut = executor.Submit(sfunc2, 1);

      EXPECT_EQ(1, fut.get());
      //==========================================
      // test w/ call Operator class
      fut = executor.Submit(Op2(val), 1);

      EXPECT_EQ(1, fut.get());

      // test submitting as a Runnable
      val = 0;
      executor.Execute(Op2(val), 1);
      EXPECT_TRUE(waitFor([&val]() { return val == 1; }, TIMEOUT));
      val = 0;
      //==========================================
      // test w/ call Operator class as lvalue
      Op2 op(val);
      fut = executor.Submit(std::move(op), 1);

      EXPECT_EQ(1, fut.get());

      // test submitting as a Runnable
      Op2 op2(val);
      val = 0;
      executor.Execute(std::move(op2), 1);
      EXPECT_TRUE(waitFor([&val]() { return val == 1; }, TIMEOUT));
      val = 0;
      //==========================================

      executor.Shutdown();
      EXPECT_TRUE(executor.IsShutdown());

      bool caught = false;
      try {
        executor.Submit([](){ return 0; });
      } catch (ExecutorShutdownExecption &e) {
        caught = true;
      }
      EXPECT_TRUE(caught);
      EXPECT_TRUE(waitFor([&executor]() { return executor.IsTerminated(); }, TIMEOUT));
    }

    TEST(TestExecutor, SimpleCallableTest)
    {
      ExecutorService<SimpleSyncronousExecutor> executor;
      testCallable(executor);
      ExecutorService<DynamicThreadExecutor> de;
      testCallable(de);
      ExecutorService<ThreadPoolExecutor, int> tpes1(1);
      testCallable(tpes1);
      ExecutorService<ThreadPoolExecutor, int> tpes(10);
      testCallable(tpes);

      ExecutorService<SimpleSyncronousExecutor> executor2;
      ExecutorService<SequentialFeedExecutor, Executor*> sfe01(&executor2);
      testCallable(sfe01);

      ExecutorService<DynamicThreadExecutor> de2;
      ExecutorService<SequentialFeedExecutor,Executor*> sfe02(&de2);
      testCallable(sfe02);


      ExecutorService<ThreadPoolExecutor, int> tpes2(10);
      ExecutorService<SequentialFeedExecutor,Executor*> sfe1(&tpes2);
      testCallable(sfe1);

      ExecutorService<ThreadPoolExecutor, int> tpes3(1);
      ExecutorService<SequentialFeedExecutor,Executor*> sfe2(&tpes3);
      testCallable(sfe2);
    }

    TEST(TestExecutor, SequentialFeedExecutorTest) {
      ExecutorService<ThreadPoolExecutor, int> tpe(100);
      ExecutorService<SequentialFeedExecutor, Executor*> se(&tpe);

      std::atomic_int index(0);
      int* val = new int[1000000];

      for (int i = 0; i < 1000000; i++)
        se.Execute([&index, val, i]() { int li = index.fetch_add(1); val[i] = li; });

      se.Shutdown();
      tpe.Shutdown();

      EXPECT_TRUE(waitFor([&tpe]() { return tpe.IsTerminated(); }, TIMEOUT));

      for (int i = 0; i < 1000000; i++)
        EXPECT_EQ(i, val[i]);

      delete [] val;
    }
  }
}
