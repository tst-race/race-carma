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

AC_DEFUN_ONCE([DEFINE_WITH_OPENSSL_SSL], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_LIBS])
  GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])
  GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])
  GATBPS_REQUIRE([DEFINE_WITH_OPENSSL_CRYPTO])
  GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])

  GATBPS_FINISH_WYNA(
    [--with-openssl-ssl],
    [WYNA_OPENSSL_SSL])

  GATBPS_PUSH_VAR([LIBS], ["-lssl $][{LIBS-}"])

  m4_pushdef(
    [prologue],
    [[
      #include <openssl/ssl.h>
    ]])

  m4_pushdef(
    [body],
    [[
      SSL_CTX_free(0);
    ]])

  AC_LANG_PUSH([C])

  GATBPS_CHECK_LINK(
    [for the OpenSSL ssl library (C link)],
    [WITH_OPENSSL_SSL_C_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
      && !WYNA_OPENSSL_SSL_IS_NO
      && WITH_OPENSSL_CRYPTO
    ])

  GATBPS_CHECK_RUN(
    [for the OpenSSL ssl library (C run)],
    [WITH_OPENSSL_SSL_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_OPENSSL_SSL_C_LINK])

  AC_LANG_POP([C])

  AC_LANG_PUSH([C++])

  GATBPS_CHECK_LINK(
    [for the OpenSSL ssl library (C++ link)],
    [WITH_OPENSSL_SSL_CPP_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
      && !WYNA_OPENSSL_SSL_IS_NO
      && WITH_OPENSSL_CRYPTO
    ])

  GATBPS_CHECK_RUN(
    [for the OpenSSL ssl library (C++ run)],
    [WITH_OPENSSL_SSL_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_OPENSSL_SSL_CPP_LINK])

  AC_LANG_POP([C++])

  m4_popdef([body])
  m4_popdef([prologue])

  GATBPS_CHECK_EXPR(
    [whether to use the OpenSSL ssl library],
    [WITH_OPENSSL_SSL],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_OPENSSL_SSL_C_RUN
        || (1
          && WYNA_OPENSSL_SSL_IS_YES
          && WITH_OPENSSL_SSL_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_OPENSSL_SSL_CPP_RUN
        || (1
          && WYNA_OPENSSL_SSL_IS_YES
          && WITH_OPENSSL_SSL_CPP_RUN_is_guess
        )
      )
    ])

  [

    case $][{WYNA_OPENSSL_SSL_IS_YES?}$][{WITH_OPENSSL_SSL?} in 10)
      ]GATBPS_BARF([
        The OpenSSL ssl library is not available.
      ])[
    esac

    if $][{WITH_OPENSSL_SSL_sh?}; then
      ]GATBPS_KEEP_VAR([LIBS])[
    else
      ]GATBPS_POP_VAR([LIBS])[
    fi

  ]

}])
