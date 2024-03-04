dnl
dnl Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
dnl
dnl Licensed under the Apache License, Version 2.0 (the "License");
dnl you may not use this file except in compliance with the License.
dnl You may obtain a copy of the License at
dnl
dnl     http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing,
dnl software distributed under the License is distributed on an "AS
dnl IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
dnl express or implied. See the License for the specific language
dnl governing permissions and limitations under the License.
dnl
dnl SPDX-License-Identifier: Apache-2.0
dnl

AC_DEFUN_ONCE([DEFINE_WITH_PTHREADS], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_BEFORE([$0], [DEFINE_CFLAGS])[
  ]GATBPS_BEFORE([$0], [DEFINE_CXXFLAGS])[

  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_HAVE_ANDROID])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]GATBPS_FINISH_WYNA(
    [--with-pthreads],
    [WYNA_PTHREADS])[

  case $][{HAVE_ANDROID-0} in 0)
    ]GATBPS_PUSH_VAR(
      [CFLAGS],
      ["-pthread $][{CFLAGS-}"])[
    ]GATBPS_PUSH_VAR(
      [CXXFLAGS],
      ["-pthread $][{CXXFLAGS-}"])[
  esac

  ]m4_pushdef(
    [prologue],
    [[
      #include <pthread.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      (void)pthread_self();
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for POSIX threads (C link)],
    [WITH_PTHREADS_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_PTHREADS_IS_NO
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for POSIX threads (C run)],
    [WITH_PTHREADS_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_PTHREADS_C_LINK])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for POSIX threads (C++ link)],
    [WITH_PTHREADS_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_PTHREADS_IS_NO
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for POSIX threads (C++ run)],
    [WITH_PTHREADS_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_PTHREADS_CPP_LINK])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [whether to use POSIX threads],
    [WITH_PTHREADS],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_PTHREADS_C_RUN
        || (1
          && WYNA_PTHREADS_IS_YES
          && WITH_PTHREADS_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_PTHREADS_CPP_RUN
        || (1
          && WYNA_PTHREADS_IS_YES
          && WITH_PTHREADS_CPP_RUN_is_guess
        )
      )
    ])[

  x=
  x=$][{x?}$][{WYNA_PTHREADS_IS_YES?}
  x=$][{x?}$][{WITH_PTHREADS?}

  case $][{x?} in 10)
    ]GATBPS_BARF([
      POSIX threads are not available.
    ])[
  ;; ?1)
    case $][{HAVE_ANDROID-0} in 0)
      ]GATBPS_KEEP_VAR([CXXFLAGS])[
      ]GATBPS_KEEP_VAR([CFLAGS])[
    esac
  ;; *)
    case $][{HAVE_ANDROID-0} in 0)
      ]GATBPS_POP_VAR([CXXFLAGS])[
      ]GATBPS_POP_VAR([CFLAGS])[
    esac
  esac

}]])
