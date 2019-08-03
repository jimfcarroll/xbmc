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
#include "threads/ConcurrentDeque.h"
#include <thread>

#include "threads/test/TestHelpers.h"
#include <chrono>

#define TIMEOUT 10000

namespace XbmcThreads
{
  namespace test
  {
    TEST(TestConcurrency, SimpleConcurrentDeque)
    {
      ConcurrentDeque<int> d;

      d.put(1);
      d.put(2);
      d.put(3);
      d.put(4);
      d.put(5);

      EXPECT_EQ(1, d.take());
      EXPECT_EQ(2, d.take());
      EXPECT_EQ(3, d.take());
      EXPECT_EQ(4, d.take());
      EXPECT_EQ(5, d.take());
    }

    TEST(TestConcurrency, ConcurrentDequeOneToMany)
    {
      ConcurrentDeque<int> d;
      int* vals= new int[10];

      std::thread** threads = new std::thread*[10];

      for (int i = 0; i < 10; i++) {
        vals[i] = 0;
        threads[i] = new std::thread([&d, vals]() {
          int val = d.take();
          vals[val]++;
        });
      }

      for (int i = 0; i < 10; i++)
        d.put(i);

      for (int i = 0; i < 10; i++) {
        threads[i]->join();
        delete threads[i];
      }

      delete [] threads;

      for (int i = 0; i < 10; i++)
        EXPECT_EQ(1, vals[i]);
    }

    TEST(TestConcurrency, ConcurrentDequeManyToOne)
    {
      ConcurrentDeque<int> d;
      int* vals= new int[10];

      std::thread** threads = new std::thread*[10];

      std::atomic_bool flag(false);

      for (int i = 0; i < 10; i++) {
        vals[i] = 0;
        threads[i] = new std::thread([&d, &flag](int index) {
          while (!flag);
          d.put(index);
        }, i);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      flag.store(true);

      for (int i = 0; i < 10; i++)
        vals[d.take()] ++;

      for (int i = 0; i < 10; i++) {
        threads[i]->join();
        delete threads[i];
      }

      delete [] threads;

      for (int i = 0; i < 10; i++)
        EXPECT_EQ(1, vals[i]);
    }

    TEST(TestConcurrency, ConcurrentDequeManyToMany)
    {
      ConcurrentDeque<int> d;
      int* vals= new int[100];

      std::thread** pubThreads = new std::thread*[10];
      std::thread** subThreads = new std::thread*[10];

      std::atomic_bool flag(false);
      for (int i = 0; i < 100; i++)
        vals[i] = 0;

      for (int i = 0; i < 10; i++) {
        subThreads[i] = new std::thread([&d, vals]() {
          for (int i  = 0; i < 10; i++) {
            int val = d.take();
            vals[val]++;
          }
        });
      }


      for (int i = 0; i < 10; i++) {
        pubThreads[i] = new std::thread([&d, &flag](int index) {
          while (!flag);
          int start = index * 10;
          for (int i = start; i < start + 10; i++)
            d.put(i);
        }, i);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      flag.store(true);

      for (int i = 0; i < 10; i++) {
        pubThreads[i]->join();
        subThreads[i]->join();
        delete pubThreads[i];
        delete subThreads[i];
      }

      delete [] pubThreads;
      delete [] subThreads;

      for (int i = 0; i < 100; i++)
        EXPECT_EQ(1, vals[i]);
    }

    TEST(TestConcurrency, ConcurrentDequeManyToManyPoll)
    {
      ConcurrentDeque<int> d;
      int* vals= new int[100];

      std::thread** pubThreads = new std::thread*[10];
      std::thread** subThreads = new std::thread*[10];

      std::atomic_bool flag(false);
      for (int i = 0; i < 100; i++)
        vals[i] = 0;

      for (int i = 0; i < 10; i++) {
        subThreads[i] = new std::thread([&d, vals]() {
          for (int i  = 0; i < 10; i++) {
            for (bool done = false; !done; ) {
              optional<int> result = d.poll();
              if (result.has_value()) {
                done = true;
                vals[result.value()]++;
              }
            }
          }
        });
      }


      for (int i = 0; i < 10; i++) {
        pubThreads[i] = new std::thread([&d, &flag](int index) {
          while (!flag);
          int start = index * 10;
          for (int i = start; i < start + 10; i++)
            d.put(i);
        }, i);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      flag.store(true);

      for (int i = 0; i < 10; i++) {
        pubThreads[i]->join();
        subThreads[i]->join();
        delete pubThreads[i];
        delete subThreads[i];
      }

      delete [] pubThreads;
      delete [] subThreads;

      for (int i = 0; i < 100; i++)
        EXPECT_EQ(1, vals[i]);
    }
  }
}
