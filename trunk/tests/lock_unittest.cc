// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simple-platform-lib/src/lock.h"

#include <gtest/gtest.h>
#include <stdlib.h>

#include "simple-platform-lib/src/thread.h"

typedef testing::Test LockTest;

// Basic test to make sure that Acquire()/Release()/Try() don't crash ----------

class BasicLockTestThread : public platform::Thread::Delegate {
 public:
  BasicLockTestThread(platform::Lock* lock) : lock_(lock), acquired_(0) {}

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

  DISALLOW_COPY_AND_ASSIGN(BasicLockTestThread);
};

TEST_F(LockTest, Basic) {
  platform::Lock lock;
  BasicLockTestThread thread(&lock);
  platform::ThreadHandle handle = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));

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

  platform::Thread::Join(handle);

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.acquired(), 20);
}

// Test that Try() works as expected -------------------------------------------

class TryLockTestThread : public platform::Thread::Delegate {
 public:
  TryLockTestThread(platform::Lock* lock) : lock_(lock), got_lock_(false) {}

  virtual void ThreadMain() {
    got_lock_ = lock_->Try();
    if (got_lock_)
      lock_->Release();
  }

  bool got_lock() const { return got_lock_; }

 private:
  platform::Lock* lock_;
  bool got_lock_;

  DISALLOW_COPY_AND_ASSIGN(TryLockTestThread);
};

TEST_F(LockTest, TryLock) {
  platform::Lock lock;

  ASSERT_TRUE(lock.Try());
  // We now have the lock....

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);
    platform::ThreadHandle handle = platform::kNullThreadHandle;

    ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));

    platform::Thread::Join(handle);

    ASSERT_FALSE(thread.got_lock());
  }

  lock.Release();

  // This thread will....
  {
    TryLockTestThread thread(&lock);
    platform::ThreadHandle handle = platform::kNullThreadHandle;

    ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));

    platform::Thread::Join(handle);

    ASSERT_TRUE(thread.got_lock());
    // But it released it....
    ASSERT_TRUE(lock.Try());
  }

  lock.Release();
}

// Tests that locks actually exclude -------------------------------------------

class MutexLockTestThread : public platform::Thread::Delegate {
 public:
  MutexLockTestThread(platform::Lock* lock, int* value)
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

  DISALLOW_COPY_AND_ASSIGN(MutexLockTestThread);
};

TEST_F(LockTest, MutexTwoThreads) {
  platform::Lock lock;
  int value = 0;

  MutexLockTestThread thread(&lock, &value);
  platform::ThreadHandle handle = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));

  MutexLockTestThread::DoStuff(&lock, &value);

  platform::Thread::Join(handle);

  EXPECT_EQ(2 * 40, value);
}

TEST_F(LockTest, MutexFourThreads) {
  platform::Lock lock;
  int value = 0;

  MutexLockTestThread thread1(&lock, &value);
  MutexLockTestThread thread2(&lock, &value);
  MutexLockTestThread thread3(&lock, &value);
  platform::ThreadHandle handle1 = platform::kNullThreadHandle;
  platform::ThreadHandle handle2 = platform::kNullThreadHandle;
  platform::ThreadHandle handle3 = platform::kNullThreadHandle;

  ASSERT_TRUE(platform::Thread::Create(0, &thread1, &handle1));
  ASSERT_TRUE(platform::Thread::Create(0, &thread2, &handle2));
  ASSERT_TRUE(platform::Thread::Create(0, &thread3, &handle3));

  MutexLockTestThread::DoStuff(&lock, &value);

  platform::Thread::Join(handle1);
  platform::Thread::Join(handle2);
  platform::Thread::Join(handle3);

  EXPECT_EQ(4 * 40, value);
}
