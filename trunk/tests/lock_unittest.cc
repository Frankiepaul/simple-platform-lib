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
  BasicLockTestThreadDelegate(platform::Lock* lock)
      : lock_(lock), acquired_(0) {}

  virtual void ThreadMain() {
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      platform::Thread::Sleep(rand() % 20);
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_->Try()) {
        acquired_++;
        platform::Thread::Sleep(rand() % 20);
        lock_->Release();
      }
    }
  }

  int acquired() const { return acquired_; }

 private:
  platform::Lock* lock_;
  int acquired_;

  DISALLOW_COPY_AND_ASSIGN(BasicLockTestThreadDelegate);
};

TEST_F(LockTest, Basic) {
  platform::Lock lock;
  BasicLockTestThreadDelegate delegate(&lock);
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

// Tests that locks actually exclude -------------------------------------------

class MutexLockTestThreadDelegate : public platform::Thread::Delegate {
 public:
  MutexLockTestThreadDelegate(platform::Lock* lock, int* value)
      : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void DoStuff(platform::Lock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      lock->Acquire();
      int v = *value;
      platform::Thread::Sleep(rand() % 10);
      *value = v + 1;
      lock->Release();
    }
  }

  virtual void ThreadMain() {
    DoStuff(lock_, value_);
  }

 private:
  platform::Lock* lock_;
  int* value_;

  DISALLOW_COPY_AND_ASSIGN(MutexLockTestThreadDelegate);
};

TEST_F(LockTest, MutexTwoThreads) {
  platform::Lock lock;
  int value = 0;

  MutexLockTestThreadDelegate delegate(&lock, &value);
  platform::ThreadHandle thread = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &delegate, &thread));

  MutexLockTestThreadDelegate::DoStuff(&lock, &value);

  platform::Thread::Join(thread);

  EXPECT_EQ(2 * 40, value);
}

TEST_F(LockTest, MutexFourThreads) {
  platform::Lock lock;
  int value = 0;

  MutexLockTestThreadDelegate delegate1(&lock, &value);
  MutexLockTestThreadDelegate delegate2(&lock, &value);
  MutexLockTestThreadDelegate delegate3(&lock, &value);
  platform::ThreadHandle thread1 = platform::kNullThreadHandle;
  platform::ThreadHandle thread2 = platform::kNullThreadHandle;
  platform::ThreadHandle thread3 = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &delegate1, &thread1));
  ASSERT_TRUE(platform::Thread::Create(0, &delegate2, &thread2));
  ASSERT_TRUE(platform::Thread::Create(0, &delegate3, &thread3));

  MutexLockTestThreadDelegate::DoStuff(&lock, &value);

  platform::Thread::Join(thread1);
  platform::Thread::Join(thread2);
  platform::Thread::Join(thread3);

  EXPECT_EQ(4 * 40, value);
}
