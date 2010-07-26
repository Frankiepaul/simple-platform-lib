// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simple-platform-lib/src/thread.h"

#include <gtest/gtest.h>

typedef testing::Test ThreadTest;

// Trivial test that thread runs and doesn't crash on create and join ----------

class TrivialThreadDelegate : public platform::thread::Delegate {
 public:
  TrivialThreadDelegate() : did_run_(false) {}

  virtual void ThreadMain() {
    did_run_ = true;
  }

  bool did_run() const { return did_run_; }

 private:
  bool did_run_;

  DISALLOW_COPY_AND_ASSIGN(TrivialThreadDelegate);
};

TEST_F(ThreadTest, TrivialThread) {
  TrivialThreadDelegate delegate;
  platform::ThreadHandle thread = platform::kNullThreadHandle;

  ASSERT_FALSE(delegate.did_run());
  ASSERT_TRUE(platform::thread::Create(0, &delegate, &thread));
  platform::thread::Join(thread);
  ASSERT_TRUE(delegate.did_run());
}

// Test of basic thread functions ----------------------------------------------

class FunctionTestThreadDelegate : public TrivialThreadDelegate {
 public:
  FunctionTestThreadDelegate() {}

  virtual void ThreadMain() {
    platform::thread::CurrentId();
    platform::thread::Yield();
    platform::thread::Sleep(100);

    TrivialThreadDelegate::ThreadMain();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(FunctionTestThreadDelegate);
};

TEST_F(ThreadTest, FunctionTestThread) {
  FunctionTestThreadDelegate delegate;
  platform::ThreadHandle thread = platform::kNullThreadHandle;

  ASSERT_FALSE(delegate.did_run());
  ASSERT_TRUE(platform::thread::Create(0, &delegate, &thread));
  platform::thread::Join(thread);
  ASSERT_TRUE(delegate.did_run());
}
