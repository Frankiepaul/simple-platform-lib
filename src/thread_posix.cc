// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Taken from Chromium: src/base/platform_thread_posix.cc
// Significant changes (other than naming):
//  - see thread.h

#include "simple-platform-lib/src/thread.h"

#include <dlfcn.h>
#include <errno.h>
#include <sched.h>

#if defined(OS_MACOSX)
#include <mach/mach.h>
#include <sys/resource.h>
#include <algorithm>
#endif

#if defined(OS_LINUX)
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace platform {
namespace Thread {

static void* ThreadFunc(void* closure) {
  Delegate* delegate = static_cast<Delegate*>(closure);
  delegate->ThreadMain();
  return NULL;
}

ThreadId CurrentId() {
  // Pthreads doesn't have the concept of a thread ID, so we have to reach down
  // into the kernel.
#if defined(OS_MACOSX)
  return mach_thread_self();
#elif defined(OS_LINUX)
  return syscall(__NR_gettid);
#elif defined(OS_FREEBSD)
  // TODO(BSD): find a better thread ID
  return reinterpret_cast<int64>(pthread_self());
#endif
}

void Yield() {
  sched_yield();
}

void Sleep(int duration_ms) {
  struct timespec sleep_time, remaining;

  // Contains the portion of duration_ms >= 1 sec.
  sleep_time.tv_sec = duration_ms / 1000;
  duration_ms -= sleep_time.tv_sec * 1000;

  // Contains the portion of duration_ms < 1 sec.
  sleep_time.tv_nsec = duration_ms * 1000 * 1000;  // nanoseconds.

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

static bool CreateThread(size_t stack_size, bool joinable, Delegate* delegate,
                         ThreadHandle* thread_handle) {
  bool success = false;
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);

  // Pthreads are joinable by default, so only specify the detached attribute if
  // the thread should be non-joinable.
  if (!joinable) {
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
  }

#if defined(OS_MACOSX)
  // The Mac OS X default for a pthread stack size is 512kB.
  // Libc-594.1.4/pthreads/pthread.c's pthread_attr_init uses
  // DEFAULT_STACK_SIZE for this purpose.
  //
  // 512kB isn't quite generous enough for some deeply recursive threads that
  // otherwise request the default stack size by specifying 0. Here, adopt
  // glibc's behavior as on Linux, which is to use the current stack size
  // limit (ulimit -s) as the default stack size. See
  // glibc-2.11.1/nptl/nptl-init.c's __pthread_initialize_minimal_internal. To
  // avoid setting the limit below the Mac OS X default or the minimum usable
  // stack size, these values are also considered. If any of these values
  // can't be determined, or if stack size is unlimited (ulimit -s unlimited),
  // stack_size is left at 0 to get the system default.
  //
  // Mac OS X normally only applies ulimit -s to the main thread stack. On
  // contemporary OS X and Linux systems alike, this value is generally 8MB
  // or in that neighborhood.
  if (stack_size == 0) {
    size_t default_stack_size;
    struct rlimit stack_rlimit;
    if (pthread_attr_getstacksize(&attributes, &default_stack_size) == 0 &&
        getrlimit(RLIMIT_STACK, &stack_rlimit) == 0 &&
        stack_rlimit.rlim_cur != RLIM_INFINITY) {
      stack_size = std::max(std::max(default_stack_size,
                                     static_cast<size_t>(PTHREAD_STACK_MIN)),
                            static_cast<size_t>(stack_rlimit.rlim_cur));
    }
  }
#endif  // OS_MACOSX

  if (stack_size > 0)
    pthread_attr_setstacksize(&attributes, stack_size);

  success = !pthread_create(thread_handle, &attributes, ThreadFunc, delegate);

  pthread_attr_destroy(&attributes);
  return success;
}

bool Create(size_t stack_size, Delegate* delegate,
            ThreadHandle* thread_handle) {
  return CreateThread(stack_size, true /* joinable thread */,
                      delegate, thread_handle);
}

bool CreateNonJoinable(size_t stack_size, Delegate* delegate) {
  ThreadHandle unused;

  bool result = CreateThread(stack_size, false /* non-joinable thread */,
                             delegate, &unused);
  return result;
}

void Join(ThreadHandle thread_handle) {
  pthread_join(thread_handle, NULL);
}

}  // namespace Thread
}  // namespace platform
