// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simple-platform-lib/src/lock.h"

#include <gtest/gtest.h>
#include <stdlib.h>

#include "simple-platform-lib/src/thread.h"

typedef testing::Test LockTest;

// Basic test to make sure that Acquire()/Release()/Try() don't crash ----------

class BasicLockTestThreadDelegate : public platform::Thread::Delegate {
 public:
  BasicLockTestThreadDelegate(platform::Lock& lock)
      : lock_(lock), acquired_(0) {}

  virtual void ThreadMain() {
    for (int i = 0; i < 10; i++) {
      lock_.Acquire();
      acquired_++;
      lock_.Release();
    }
    for (int i = 0; i < 10; i++) {
      lock_.Acquire();
      acquired_++;
      platform::Thread::Sleep(rand() % 20);
      lock_.Release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_.Try()) {
        acquired_++;
        platform::Thread::Sleep(rand() % 20);
        lock_.Release();
      }
    }
  }

  int acquired() const { return acquired_; }

 private:
  platform::Lock& lock_;
  int acquired_;

  DISALLOW_COPY_AND_ASSIGN(BasicLockTestThreadDelegate);
};

TEST_F(LockTest, Basic) {
  platform::Lock lock;
  BasicLockTestThreadDelegate delegate(lock);
  platform::ThreadHandle thread = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &delegate, &thread));

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    lock.Acquire();
    acquired++;
    platform::Thread::Sleep(rand() % 20);
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.Try()) {
      acquired++;
      platform::Thread::Sleep(rand() % 20);
      lock.Release();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    platform::Thread::Sleep(rand() % 20);
    lock.Release();
  }

  platform::Thread::Join(thread);

  EXPECT_GE(acquired, 20);
  EXPECT_GE(delegate.acquired(), 20);
}
