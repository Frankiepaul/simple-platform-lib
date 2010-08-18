// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simple-platform-lib/src/thread.h"

#include <gtest/gtest.h>

typedef testing::Test ThreadTest;

// Trivial test that thread runs and doesn't crash on create and join ----------

class TrivialThread : public platform::Thread::Delegate {
 public:
  TrivialThread() : did_run_(false) {}

  virtual void ThreadMain() {
    did_run_ = true;
  }

  bool did_run() const { return did_run_; }

 private:
  bool did_run_;

  DISALLOW_COPY_AND_ASSIGN(TrivialThread);
};

TEST_F(ThreadTest, Trivial) {
  TrivialThread thread;
  platform::ThreadHandle handle = platform::kNullThreadHandle;

  ASSERT_FALSE(thread.did_run());
  ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));
  platform::Thread::Join(handle);
  ASSERT_TRUE(thread.did_run());
}

TEST_F(ThreadTest, TrivialTimesTen) {
  TrivialThread thread[10];
  platform::ThreadHandle handle[arraysize(thread)];

  for (size_t n = 0; n < arraysize(thread); n++)
    ASSERT_FALSE(thread[n].did_run());
  for (size_t n = 0; n < arraysize(thread); n++)
    ASSERT_TRUE(platform::Thread::Create(0, &thread[n], &handle[n]));
  for (size_t n = 0; n < arraysize(thread); n++)
    platform::Thread::Join(handle[n]);
  for (size_t n = 0; n < arraysize(thread); n++)
    ASSERT_TRUE(thread[n].did_run());
}

// Test of basic thread functions ----------------------------------------------

class FunctionTestThread : public TrivialThread {
 public:
  FunctionTestThread() : thread_id_(0) {}

  virtual void ThreadMain() {
    thread_id_ = platform::Thread::CurrentId();
    platform::Thread::CurrentId();
    platform::Thread::Yield();
    platform::Thread::Sleep(50);

    TrivialThread::ThreadMain();
  }

  platform::ThreadId thread_id() const { return thread_id_; }

 private:
  platform::ThreadId thread_id_;

  DISALLOW_COPY_AND_ASSIGN(FunctionTestThread);
};

TEST_F(ThreadTest, Function) {
  platform::ThreadId main_thread_id = platform::Thread::CurrentId();

  FunctionTestThread thread;
  platform::ThreadHandle handle = platform::kNullThreadHandle;

  ASSERT_FALSE(thread.did_run());
  ASSERT_TRUE(platform::Thread::Create(0, &thread, &handle));
  platform::Thread::Join(handle);
  ASSERT_TRUE(thread.did_run());
  EXPECT_NE(thread.thread_id(), main_thread_id);
}

TEST_F(ThreadTest, FunctionTimesTen) {
  platform::ThreadId main_thread_id = platform::Thread::CurrentId();

  FunctionTestThread thread[10];
  platform::ThreadHandle handle[arraysize(thread)];

  for (size_t n = 0; n < arraysize(thread); n++)
    ASSERT_FALSE(thread[n].did_run());
  for (size_t n = 0; n < arraysize(thread); n++)
    ASSERT_TRUE(platform::Thread::Create(0, &thread[n], &handle[n]));
  for (size_t n = 0; n < arraysize(thread); n++)
    platform::Thread::Join(handle[n]);
  for (size_t n = 0; n < arraysize(thread); n++) {
    ASSERT_TRUE(thread[n].did_run());
    EXPECT_NE(thread[n].thread_id(), main_thread_id);
  }
}
