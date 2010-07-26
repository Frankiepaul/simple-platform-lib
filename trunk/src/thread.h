// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Taken from Chromium: src/base/platform_thread.h
// Significant changes (other than naming):
//  - namespace |base| -> |platform|
//  - class |Thread| (with only static methods) -> namespace |thread|
//  - |YieldCurrentThread()| -> |Yield()|
//  - |SetName()| removed
//  - (Mac) |InitThreading()| removed, so if Cocoa is going to be used, it must
//    be warmed up (see Chromium: src/base/platform_thread_mac.mm)

// This class provides a low-level platform-specific abstraction to the OS's
// threading interface, on top of which application-level abstractions can be
// built.

#ifndef SIMPLEPLATFORMLIB_SRC_PLATFORM_THREAD_H_
#define SIMPLEPLATFORMLIB_SRC_PLATFORM_THREAD_H_

#include "simple-platform-lib/src/basictypes.h"

// ThreadHandle should not be assumed to be a numeric type, since the standard
// intends to allow pthread_t to be a structure.  This means you should not
// initialize it to a value, like 0.  If it's a member variable, the constructor
// can safely "value initialize" using () in the initializer list.
#if defined(OS_WIN)
#include <windows.h>
namespace platform {
typedef DWORD ThreadId;
typedef void* ThreadHandle;  // HANDLE
const ThreadHandle kNullThreadHandle = NULL;
}  // namespace platform
#elif defined(OS_POSIX)
#include <pthread.h>
namespace platform {
typedef pthread_t ThreadHandle;
const ThreadHandle kNullThreadHandle = 0;
}  // namespace platform
#if defined(OS_MACOSX)
#include <mach/mach.h>
namespace platform {
typedef mach_port_t ThreadId;
}  // namespace platform
#else  // OS_POSIX && !OS_MACOSX
#include <unistd.h>
namespace platform {
typedef pid_t ThreadId;
}  // namespace platform
#endif
#endif

namespace platform {
namespace thread {

// Gets the current thread id, which may be useful for logging purposes.
ThreadId CurrentId();

// Yield the current thread so another thread can be scheduled.
void Yield();

// Sleeps for the specified duration (units are milliseconds).
void Sleep(int duration_ms);

// Implement this interface to run code on a background thread.  Your
// ThreadMain method will be called on the newly created thread.
class Delegate {
 public:
  virtual ~Delegate() {}
  virtual void ThreadMain() = 0;
};

// Creates a new thread.  The |stack_size| parameter can be 0 to indicate that
// the default stack size should be used.  Upon success, |*thread_handle| will
// be assigned a handle to the newly created thread, and |delegate|'s ThreadMain
// method will be executed on the newly created thread.

// NOTE: When you are done with the thread handle, you must call Join to release
// system resources associated with the thread.  You must ensure that the
// Delegate object outlives the thread.
bool Create(size_t stack_size, Delegate* delegate, ThreadHandle* thread_handle);

// CreateNonJoinable() does the same thing as Create() except the thread cannot
// be Join()'d.  Therefore, it also does not output a ThreadHandle.
bool CreateNonJoinable(size_t stack_size, Delegate* delegate);

// Joins with a thread created via the Create function.  This function blocks
// the caller until the designated thread exits.  This will invalidate
// |thread_handle|.
void Join(ThreadHandle thread_handle);

}  // namespace thread
}  // namespace platform

#endif  // SIMPLEPLATFORMLIB_SRC_PLATFORM_THREAD_H_
