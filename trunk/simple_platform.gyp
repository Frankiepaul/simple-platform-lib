# Copyright (c) 2010 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,  # Use higher warning level.
  },
  'target_defaults': {
    'conditions': [
      # Linux shared libraries should always be built -fPIC.
      ['OS=="linux" or OS=="openbsd" or OS=="freebsd" or OS=="solaris"', {
        'cflags': ['-fPIC', '-fvisibility=hidden'],

        # This is needed to make the Linux shlib build happy. Without this,
        # -fvisibility=hidden gets stripped by the exclusion in common.gypi
        # that is triggered when a shared library build is specified.
        'cflags/': [['include', '^-fvisibility=hidden$']],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'simple_platform',
      'type': 'static_library',
      'msvs_guid': '559673B9-D26C-4DA2-8C52-545F80BBD701',
      #'dependencies': [
      #],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'src/basictypes.h',
        'src/lock.cc',
        'src/lock.h',
        'src/lock_impl.h',
        'src/lock_impl_posix.cc',
        'src/port.h',
        'src/thread.h',
        'src/thread_posix.cc',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'AdditionalOptions': ['/we4244'],  # implicit conversion, possible loss of data
            },
          },
        }],
        ['OS=="linux"', {
#          'cflags': ['-Wextra', '-pedantic'],
          'cflags': ['-Wextra', ],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
#            'WARNING_CFLAGS': ['-Wextra', '-pedantic'], 
            'WARNING_CFLAGS': ['-Wextra', ], 
           },
        }]
      ],
    },
    {
      'target_name': 'simple_platform_unittests',
      'type': 'executable',
      'msvs_guid': '8152F3E4-85E8-4712-A14E-2C9191D1896D',
      'include_dirs': [
        '..',
      ],
      'sources': [
        # Test runner.
        'tests/unittest_main.cc',

        # Tests.
        'tests/lock_unittest.cc',
        'tests/thread_unittest.cc',
      ],
      'dependencies': [
        'simple_platform',
        'gtest/gtest.gyp:gtest',
      ],
    },
  ],
}
