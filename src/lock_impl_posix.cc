// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Taken from Chromium: src/base/lock_impl_posix.cc

#include "simple-platform-lib/src/lock_impl.h"

#include <errno.h>

//FIXME
//#include "base/logging.h"

namespace platform {

LockImpl::LockImpl() {
#ifndef NDEBUG
  // In debug, setup attributes for lock error checking.
  pthread_mutexattr_t mta;
  int rv = pthread_mutexattr_init(&mta);
//  DCHECK_EQ(rv, 0);
  rv = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
//  DCHECK_EQ(rv, 0);
  rv = pthread_mutex_init(&os_lock_, &mta);
//  DCHECK_EQ(rv, 0);
  rv = pthread_mutexattr_destroy(&mta);
//  DCHECK_EQ(rv, 0);
(void)rv;
#else
  // In release, go with the default lock attributes.
  pthread_mutex_init(&os_lock_, NULL);
#endif
}

LockImpl::~LockImpl() {
  int rv = pthread_mutex_destroy(&os_lock_);
//  DCHECK_EQ(rv, 0);
(void)rv;
}

bool LockImpl::Try() {
  int rv = pthread_mutex_trylock(&os_lock_);
//  DCHECK(rv == 0 || rv == EBUSY);
  return rv == 0;
}

void LockImpl::Lock() {
  int rv = pthread_mutex_lock(&os_lock_);
//  DCHECK_EQ(rv, 0);
(void)rv;
}

void LockImpl::Unlock() {
  int rv = pthread_mutex_unlock(&os_lock_);
//  DCHECK_EQ(rv, 0);
(void)rv;
}

}  // namespace platform
