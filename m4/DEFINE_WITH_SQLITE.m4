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

AC_DEFUN_ONCE([DEFINE_WITH_SQLITE], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_BEFORE([$0], [DEFINE_LIBS])[

  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_LIBDL])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_LIBM])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_LIBRT])[

  ]GATBPS_FINISH_WYNA(
    [--with-sqlite],
    [WYNA_SQLITE])[

  ]GATBPS_PUSH_VAR(
    [LIBS],
    ["-lsqlite3 $][{LIBS-}"])[

  ]m4_pushdef(
    [prologue],
    [[
      #include <sqlite3.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      (void)sqlite3_libversion();
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for SQLite (C link)],
    [WITH_SQLITE_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_SQLITE_IS_NO
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for SQLite (C run)],
    [WITH_SQLITE_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_SQLITE_C_LINK])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for SQLite (C++ link)],
    [WITH_SQLITE_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_SQLITE_IS_NO
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for SQLite (C++ run)],
    [WITH_SQLITE_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_SQLITE_CPP_LINK])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [whether to use SQLite],
    [WITH_SQLITE],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_SQLITE_C_RUN
        || (1
          && WYNA_SQLITE_IS_YES
          && WITH_SQLITE_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_SQLITE_CPP_RUN
        || (1
          && WYNA_SQLITE_IS_YES
          && WITH_SQLITE_CPP_RUN_is_guess
        )
      )
    ])[

  x=
  x=$][{x?}$][{WYNA_SQLITE_IS_YES?}
  x=$][{x?}$][{WITH_SQLITE?}
  case $][{x?} in 10)
    ]GATBPS_BARF([
      SQLite is not available.
    ])[
  esac

  if $][{WITH_SQLITE_sh?}; then
    ]GATBPS_KEEP_VAR([LIBS])[
  else
    ]GATBPS_POP_VAR([LIBS])[
  fi

}]])
