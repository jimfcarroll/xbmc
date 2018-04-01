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

#include <limits.h>
#if defined(TARGET_ANDROID)
#include <unistd.h>
#else
#include <sys/syscall.h>
#endif
#include <sys/resource.h>
#include <string.h>
#ifdef TARGET_FREEBSD
#include <sys/param.h>
#include <pthread_np.h>
#endif

#include <signal.h>
#include "utils/log.h"

namespace XbmcThreads
{
  // ==========================================================
  static pthread_mutexattr_t recursiveAttr;

  static bool SetRecursiveAttr()
  {
    static bool alreadyCalled = false; // initialized to 0 in the data segment prior to startup init code running
    if (!alreadyCalled)
    {
      pthread_mutexattr_init(&recursiveAttr);
      pthread_mutexattr_settype(&recursiveAttr,PTHREAD_MUTEX_RECURSIVE);
#if !defined(TARGET_ANDROID)
      pthread_mutexattr_setprotocol(&recursiveAttr,PTHREAD_PRIO_INHERIT);
#endif
      alreadyCalled = true;
    }
    return true; // note, we never call destroy.
  }

  static bool recursiveAttrSet = SetRecursiveAttr();

  pthread_mutexattr_t* CRecursiveMutex::getRecursiveAttr()
  {
    if (!recursiveAttrSet) // this is only possible in the single threaded startup code
      recursiveAttrSet = SetRecursiveAttr();
    return &recursiveAttr;
  }
  // ==========================================================
}

// -----------------------------------------------------------------------------------
// These are platform specific and can be found in ./platform/[platform]/ThreadSchedImpl.cpp
// -----------------------------------------------------------------------------------
static bool SetPrioritySched_RR(int iPriority)
{
#if defined(TARGET_DARWIN_IOS)
  // Changing to SCHED_RR is safe under OSX, you don't need elevated privileges and the
  // OSX scheduler will monitor SCHED_RR threads and drop to SCHED_OTHER if it detects
  // the thread running away. OSX automatically does this with the CoreAudio audio
  // device handler thread.
  int32_t result;
  thread_extended_policy_data_t theFixedPolicy;

  // make thread fixed, set to 'true' for a non-fixed thread
  theFixedPolicy.timeshare = false;
  result = thread_policy_set(pthread_mach_thread_np(ThreadId()), THREAD_EXTENDED_POLICY,
    (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);

  int policy;
  struct sched_param param;
  result = pthread_getschedparam(ThreadId(), &policy, &param );
  // change from default SCHED_OTHER to SCHED_RR
  policy = SCHED_RR;
  result = pthread_setschedparam(ThreadId(), policy, &param );
  return result == 0;
#else
  return false;
#endif
}

static pid_t GetCurrentThreadPid()
{
#ifdef TARGET_FREEBSD
#if __FreeBSD_version < 900031
  long lwpid;
  thr_self(&lwpid);
  return lwpid;
#else
  return pthread_getthreadid_np();
#endif
#elif defined(TARGET_ANDROID)
  return gettid();
#else
  return syscall(SYS_gettid);
#endif
}

#ifdef RLIMIT_NICE
static int GetUserMaxPriority(int maxPriority) {
  // get user max prio
  struct rlimit limit;
  int userMaxPrio;
  if (getrlimit(RLIMIT_NICE, &limit) == 0)
  {
    userMaxPrio = limit.rlim_cur - 20;
    if (userMaxPrio < 0)
      userMaxPrio = 0;
  }
  else
    userMaxPrio = 0;

  if (geteuid() == 0)
    userMaxPrio = maxPriority;
  return userMaxPrio;
}
#endif

void CThread::SetThreadInfo()
{

#if defined(TARGET_DARWIN)
  pthread_setname_np(m_ThreadName.c_str());
#elif defined(TARGET_LINUX) && defined(__GLIBC__)
  pthread_setname_np(GetCurrentThreadNativeHandle(), m_ThreadName.c_str());
#endif

#ifdef RLIMIT_NICE
  // get user max prio
  int userMaxPrio = GetUserMaxPriority(GetMaxPriority());

  // if the user does not have an entry in limits.conf the following
  // call will fail
  if (userMaxPrio > 0)
  {
    // start thread with nice level of application
    int appNice = getpriority(PRIO_PROCESS, getpid());
    if (setpriority(PRIO_PROCESS, GetCurrentThreadPid(), appNice) != 0)
      CLog::Log(LOGERROR, "%s: error %s", __FUNCTION__, strerror(errno));
  }
#endif
}

int CThread::GetMinPriority(void)
{
  // one level lower than application
  return -1;
}

int CThread::GetMaxPriority(void)
{
  // one level higher than application
  return 1;
}

int CThread::GetNormalPriority(void)
{
  // same level as application
  return 0;
}

bool CThread::SetPriority(const int iPriority)
{
  bool bReturn = false;

  // get min prio for SCHED_RR
  int minRR = GetMaxPriority() + 1;

  pthread_t tid = static_cast<pthread_t>(GetCurrentThreadNativeHandle());

  if (!tid)
    bReturn = false;
  else if (iPriority >= minRR)
    bReturn = SetPrioritySched_RR(iPriority);
#ifdef RLIMIT_NICE
  else
  {
    // get user max prio
    int userMaxPrio = GetUserMaxPriority(GetMaxPriority());

    // keep priority in bounds
    int prio = iPriority;
    if (prio >= GetMaxPriority())
      prio = std::min(GetMaxPriority(), userMaxPrio);
    if (prio < GetMinPriority())
      prio = GetMinPriority();

    // nice level of application
    int appNice = getpriority(PRIO_PROCESS, getpid());
    if (prio)
      prio = prio > 0 ? appNice-1 : appNice+1;

    if (setpriority(PRIO_PROCESS,GetCurrentThreadPid(), prio) == 0)
      bReturn = true;
    else
      CLog::Log(LOGERROR, "%s: error %s", __FUNCTION__, strerror(errno));
  }
#endif

  return bReturn;
}

int CThread::GetPriority()
{
  int iReturn;

  int appNice = getpriority(PRIO_PROCESS, getpid());
  int prio = getpriority(PRIO_PROCESS, GetCurrentThreadPid());
  iReturn = appNice - prio;

  return iReturn;
}

int64_t CThread::GetAbsoluteUsage()
{
  CSingleLock lock(m_CriticalSection);

  if (!m_thread)
    return 0;

  int64_t time = 0;
#ifdef TARGET_DARWIN
  thread_basic_info threadInfo;
  mach_msg_type_number_t threadInfoCount = THREAD_BASIC_INFO_COUNT;

  kern_return_t ret = thread_info(pthread_mach_thread_np(m_ThreadId),
    THREAD_BASIC_INFO, (thread_info_t)&threadInfo, &threadInfoCount);

  if (ret == KERN_SUCCESS)
  {
    // User time.
    time = ((int64_t)threadInfo.user_time.seconds * 10000000L) + threadInfo.user_time.microseconds*10L;

    // System time.
    time += (((int64_t)threadInfo.system_time.seconds * 10000000L) + threadInfo.system_time.microseconds*10L);
  }

#else
  clockid_t clock;
  if (pthread_getcpuclockid(static_cast<pthread_t>(m_thread->native_handle()), &clock) == 0)
  {
    struct timespec tp;
    clock_gettime(clock, &tp);
    time = (int64_t)tp.tv_sec * 10000000 + tp.tv_nsec/100;
  }
#endif

  return time;
}

void term_handler (int signum)
{
  CLog::Log(LOGERROR,"thread 0x%lx (%lu) got signal %d. calling OnException and terminating thread abnormally.", (long unsigned int)pthread_self(), (long unsigned int)pthread_self(), signum);
  CThread* curThread = CThread::GetCurrentThread();
  if (curThread)
  {
    curThread->StopThread(false);
    curThread->OnException();
    if( curThread->IsAutoDelete() )
      delete curThread;
  }
  pthread_exit(NULL);
}

void CThread::SetSignalHandlers()
{
  struct sigaction action;
  action.sa_handler = term_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  //sigaction (SIGABRT, &action, NULL);
  //sigaction (SIGSEGV, &action, NULL);
}

