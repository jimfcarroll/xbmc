/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
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

// Should be linked as follows:
//g++ ... -o xbmc.bin -Wl,--wrap,malloc -Wl,--wrap,free -DGLIBCXX_FORCE_NEW -Wl,--wrap,calloc -Wl,--wrap,realloc -Wl,--wrap,valloc -Wl,--wrap,memalign -Wl,--wrap,posix_memalign -Wl,--wrap,_ZdaPv -Wl,--wrap,_ZdlPv -Wl,--wrap,_Znam -Wl,--wrap,_Znwm -Wl,--wrap,aligned_alloc -Wl,--wrap,pvalloc xbmc/main/main.a ...
// The pertinent section starting wth -Wl, and ending with the last -Wl. 
// To get the link line run make: 
//    V=1 make
// Take the link line, edit it as shown. Then leak detection is in.

#include "leak.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <set>
#include <vector>
#include <string>

#include "threads/SingleLock.h"
#include "threads/ThreadLocal.h"
#include "interfaces/legacy/Tuple.h"

//void* operator new (size_t size)
extern "C" void* __wrap__Znwm(size_t size)
{
 void *p=malloc(size); 
 if (p==0) // did malloc succeed?
  throw std::bad_alloc(); // ANSI/ISO compliant behavior
 return p;
}  

//void operator delete(void* del)
extern "C" void __wrap__ZdlPv(void* del)
{
  if (del != NULL)
    free(del);
}


//void* operator new[] (size_t size)
extern "C" void* __wrap__Znam(size_t size)
{
 void *p=malloc(size); 
 if (p==0) // did malloc succeed?
  throw std::bad_alloc(); // ANSI/ISO compliant behavior
 return p;
}  

// void operator delete[] (void*)
extern "C" void __wrap__ZdaPv(void* del)
{
  if (del != NULL)
    free(del);
}

// 64-bit magic number XBMCCBMX
//#define MAGIC_NUMBER 0x58626D63636D6258
//#define CLEAR_MAGIC_NUMBER (~MAGIC_NUMBER)

#define BTFRAMES 64000

extern "C" void* __real_malloc(size_t);
extern "C" void* __real_calloc(size_t,size_t);
extern "C" void* __real_realloc(void*, size_t);
extern "C" void* __real_valloc(size_t);
extern "C" void* __real_memalign(size_t,size_t);
extern "C" int __real_posix_memalign(void**,size_t,size_t);
extern "C" void __real_free(void*);
extern "C" void* __real_pvalloc(size_t);
extern "C" void* __real_aligned_alloc(size_t, size_t);

// This is called from atexit, just in case we forgot to close 
static void wrapup()
{
  printf ("Final leak detection wrapup.\n");
  XbmcCommons::EndLeakTracking();
}

struct Backtrace {
  void** bt;
  size_t frames;
  
  inline Backtrace() : bt(NULL), frames(0) {}
  inline ~Backtrace() {}
  inline Backtrace(const Backtrace& o) : bt(o.bt), frames(o.frames) {}
  inline Backtrace& operator=(const Backtrace& o) { bt=o.bt; frames = o.frames; return *this; }

  inline bool isSet() const { return bt != NULL; }

  inline void set(void* pbt[], size_t pframes) {
    frames = pframes;
    size_t numbytes = pframes * sizeof(void*);
    bt = (void**)__real_malloc(numbytes);
    for (size_t i = 0; i < pframes; i++)
      bt[i] = pbt[i];
  }

  inline void free() { if (bt != NULL) __real_free(bt); }
  void dump() const;
};

struct Allocaction
{
  Backtrace mbt;
  Backtrace fbt;
  size_t numBytes;
  void* alloc;
  unsigned long seq;

  inline Allocaction(void* palloc, size_t pnumBytes, unsigned long sequence) : numBytes(pnumBytes), alloc(palloc), seq(sequence) { }
  inline Allocaction(void* palloc) : alloc(palloc) {}
  inline Allocaction(const Allocaction& o) : mbt(o.mbt), fbt(o.fbt), numBytes(o.numBytes), alloc(o.alloc), seq(o.seq) {}

  inline bool operator<(const Allocaction& o) const { return alloc < o.alloc; }
  inline bool operator==(const Allocaction& o) const { return alloc == o.alloc; }
  inline Allocaction& operator=(const Allocaction& o) { mbt = o.mbt; fbt = o.fbt; numBytes = o.numBytes; alloc = o.alloc; seq = o.seq; return *this; }

  inline void free() { mbt.free(); fbt.free(); }

  void dump() const;
};

class LeakAdmin
{
public:
  volatile bool leakDetect;
  std::set<Allocaction> allocations;
  unsigned long curSequence;
  unsigned long mark;
  XbmcThreads::ThreadLocal<XbmcCommons::LeakDetectOff> leakDetectBlock;
  CCriticalSection ccrit;
  void* sBacktrace[BTFRAMES];
  std::vector<XBMCAddon::Tuple<unsigned long,std::string> > marks;

  inline LeakAdmin() : leakDetect(true), curSequence(0), mark((unsigned long)-1) {
    printf("Initializing Leak Detection.\n");
    memset(sBacktrace,0,(BTFRAMES * sizeof(void*)));
    atexit(wrapup);
  }

  inline void* operator new(size_t size) { return __real_malloc(size);  }
};

static inline LeakAdmin* getLeakAdmin()
{
  static LeakAdmin* instance = NULL;
  if (instance == NULL)
    instance = new LeakAdmin();
  return instance;
}

// The BSS segment will set this to true on initialization
struct InitSetter { inline InitSetter() { getLeakAdmin(); } };
static InitSetter isetter;

#define LA getLeakAdmin()

static inline bool checkForFrame(char** bt, int numFrames,int framenum, const char* check) {
  return (numFrames > framenum) && (strstr(((char*)(bt[framenum])),check) != NULL);
}

void Allocaction::dump() const {
  printf("%ld'th, %ld bytes allocated at 0x%lx\n",seq,(long)numBytes,(long)alloc);
  printf("Alloced at:\n");
  mbt.dump();
  printf("Freed at:\n");
  fbt.dump();
}

void Backtrace::dump() const {
  if (!isSet()) {
    printf("Unset backtrace\n");
    return;
  }

  char** sbt = backtrace_symbols(bt,frames);
  // filter out some things that are either not leaks, or I have no control of anyway
  if (!checkForFrame(sbt,frames,frames-3,"__libc_csu_init") && // we dont want traces from csu_init
      (!checkForFrame(sbt,frames,1,"dll_putenv")) && // nor env vars set by the initialization of Application
      (!checkForFrame(sbt,frames,frames-5,"_ZN7JSONRPC8CJSONRPC10InitializeEv"))) // This leaks because of circular shared_ptr references
  {
    for (size_t i = 0; i < frames; i++)
      printf("    %s\n",sbt[i]);
    printf("\n");
  }
  __real_free(sbt);
}

#define BACKTRACE(bt) { size_t bt__frames = backtrace(LA->sBacktrace, BTFRAMES); (bt).set(LA->sBacktrace,bt__frames); }

static void registerAlloc(void* alloc, size_t c) {
  if (XbmcCommons::LeakDetectOff::isLeakDetectEnabled() && LA->leakDetect) {
    XbmcCommons::LeakDetectOff ldo;
//    printf("My posix_memalign called with 0x%lx, %ld, %ld\n", (long)res,(long)boundary, (long)size);

    Allocaction a(alloc,c,LA->curSequence++);
    BACKTRACE(a.mbt); // fill in backtrace

    // do we already have a free allocation
    std::set<Allocaction>::iterator i = LA->allocations.find(a);
    if (i != LA->allocations.end()) {
      // we have a 'free' ... lets to a simple error check.
      const Allocaction& o = (*i);
      if (!o.fbt.isSet()) {
        printf("ERROR IN TRACKING ... MALLOC RETURNED AN ALREADY ALLOCATED BLOCK:\n");
        o.dump();
        printf("   ALLOCATED AGAIN AT:\n");
        Backtrace bt;
        BACKTRACE(bt);
        bt.dump();
      }
      LA->allocations.erase(i);
    }

    LA->allocations.insert(a);
  }
}

static unsigned char pattern[] = { 0xde, 0xad, 0xbe, 0xef };
static size_t mask = 0x3;

static void registerFree(void* f) {
  if (XbmcCommons::LeakDetectOff::isLeakDetectEnabled() && LA->leakDetect) {
    XbmcCommons::LeakDetectOff ldo;
//    printf("My free called with 0x%lx\n", (long)f);

    Allocaction a(f);
    std::set<Allocaction>::iterator i = LA->allocations.find(a);
    if (i != LA->allocations.end()) {
      a = (*i);
      for (size_t index = 0; index < a.numBytes; index++) {
        ((unsigned char*)f)[index] = pattern[index & mask];
      }
      Backtrace curFree;
      BACKTRACE(curFree);
      if (a.fbt.isSet()) {
        // double free!!!
        printf("Double Free!!!!\n");
        a.dump();
        printf("Second free pos:\n");
        curFree.dump();
        curFree.free();
      } else {
        a.fbt = curFree;
      }
      LA->allocations.erase(i);
      LA->allocations.insert(a);
    } else if (f != NULL) {
      printf("free called on unknown alloc 0x%lx\n", (long)f);
    }
  }
}

extern "C" void* __wrap_malloc(size_t c) 
{
  void* ret;
  posix_memalign(&ret,sizeof(void*),c);
  return ret;
}

extern "C" void __wrap_free(void* f) 
{
  CSingleLock lock(LA->ccrit);
  registerFree(f);
  __real_free(f);
}

extern "C" void* __wrap_calloc(size_t num, size_t size )
{
  printf("My calloc called with %ld, %ld\n", (long)num, (long)size);
  return __real_calloc(num, size);
}

extern "C" void* __wrap_valloc(size_t size )
{
  printf("My valloc called with %ld\n", (long)size);
  return __real_valloc(size);
}

extern "C" void* __wrap_memalign(size_t boundary, size_t size )
{
  printf("My memalign called with %ld, %ld\n", (long)boundary, (long)size);
  return __real_memalign(boundary, size);
}

extern "C" int __wrap_posix_memalign(void** res, size_t boundary, size_t size )
{
  CSingleLock lock(LA->ccrit);
  int status = __real_posix_memalign(res,boundary, size);
  registerAlloc(*res,size);
  return status;
}

extern "C" void* __wrap_realloc( void *memblock, size_t size )
{
  CSingleLock lock(LA->ccrit);

  // You can pass NULL to realloc and it's just a malloc
  if (memblock == NULL) return malloc(size);

  void* ret = __real_realloc(memblock, size);

  // if we changed addresses then we need to account for the new allocation
  // and clean up the old one.
  if (ret != memblock) {
    registerFree(memblock);
    registerAlloc(ret,size);
  } else {  // otherwise we need to mark the existing alloc
    CSingleLock lock(LA->ccrit);
    if (XbmcCommons::LeakDetectOff::isLeakDetectEnabled() && LA->leakDetect) {
      XbmcCommons::LeakDetectOff ldo;

      Allocaction a(memblock);
      std::set<Allocaction>::iterator i = LA->allocations.find(a);
      if (i != LA->allocations.end()) {
        a = (*i);
        a.numBytes = size;
        LA->allocations.erase(i);
        LA->allocations.insert(a);
      } else {
        printf("Unknown realloc called with 0x%lx, %ld\n", (long)memblock,(long)size);
        lock.Leave();
        registerAlloc(ret,size);
      }
    }
  }

  return ret;
}

extern "C" void* __wrap_aligned_alloc(size_t alignment, size_t size ) {
  printf("My aligned_alloc called with %ld, %ld\n", (long)alignment, (long)size);
  return __real_aligned_alloc(alignment, size);
}

extern "C" void* __wrap_pvalloc(size_t size )
{
  printf("My pvalloc called with %ld\n", (long)size);
  return __real_pvalloc(size);
}


namespace XbmcCommons
{
  void StartLeakTracking()
  {
    printf("Starting leak detection.\n");
    CSingleLock lock(LA->ccrit);
    LA->leakDetect = true;
  }

  struct SortBySequence
  {
    inline bool operator()(const Allocaction& lhs, const Allocaction& rhs)
    {
      return lhs.seq < rhs.seq;
    }
  };

  static void dumpMark(XBMCAddon::Tuple<unsigned long,std::string>& mark)
  {
    printf("===========================================================\n");
    printf("%ld: %s\n", mark.first(), mark.second().c_str());
    printf("===========================================================\n");
    printf("\n");
  }

  void DumpAllocs()
  {
    LeakDetectOff ldo;
    CSingleLock lock(LA->ccrit);
    printf("Dumping leaks detection.\n");
    std::set<Allocaction,SortBySequence> sortedSet;
    for (std::set<Allocaction>::iterator iter = LA->allocations.begin();
         iter != LA->allocations.end(); iter++)
    {
      sortedSet.insert(*iter);
    }

    std::vector<XBMCAddon::Tuple<unsigned long,std::string> >::iterator markIter = LA->marks.begin();
    XBMCAddon::Tuple<unsigned long,std::string> nextMark = markIter != LA->marks.end() ? (*markIter) : XBMCAddon::Tuple<unsigned long,std::string>((unsigned long)-1,"");
    for (std::set<Allocaction>::iterator iter = sortedSet.begin();
         iter != sortedSet.end(); iter++)
    {
      Allocaction a(*iter);
      while (a.seq > nextMark.first())
      {
        dumpMark(nextMark);
        markIter++;
        nextMark = markIter != LA->marks.end() ? (*markIter) : XBMCAddon::Tuple<unsigned long,std::string>((unsigned long)-1,"");
      }
      if (!a.fbt.isSet()) a.dump();
    }

    while (markIter != LA->marks.end())
      dumpMark(*markIter++);
  }

  void ClearAllocs()
  {
    printf("Clearing leak detection allocs.\n");
    {
      CSingleLock lock(LA->ccrit);
      for (std::set<Allocaction>::iterator iter = LA->allocations.begin();
           iter != LA->allocations.end(); iter++)
      {
        Allocaction a(*iter);
        a.free();
      }
      LA->allocations.clear();
    }
  }

  void EndLeakTracking()
  {
    printf("Ending leak detection.\n");
    {
      CSingleLock lock(LA->ccrit);
      LA->leakDetect = false;
      DumpAllocs();
      ClearAllocs();
    }
  }

  void MarkLeakTracking(const char* trackText)
  {
    CSingleLock lock(LA->ccrit);
    LeakDetectOff ldo;
    LA->marks.push_back(XBMCAddon::Tuple<unsigned long,std::string>(LA->curSequence++,std::string(trackText)));
  }

  bool IsLeakTrackingOn()
  {
    CSingleLock lock(LA->ccrit);
    return LA->leakDetect;
  }

  bool LeakDetectOff::isLeakDetectEnabled() { return LA->leakDetectBlock.get() == NULL; }

  LeakDetectOff::LeakDetectOff()
  {
    if (LA->leakDetectBlock.get() == NULL)
      LA->leakDetectBlock.set(this);
  }

  LeakDetectOff::~LeakDetectOff()
  {
    if (LA->leakDetectBlock.get() == this)
      LA->leakDetectBlock.set(NULL);
  }
}


