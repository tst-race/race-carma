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

AC_DEFUN_ONCE([DEFINE_WITH_KESTREL_RABBITMQ], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])
  GATBPS_REQUIRE([DEFINE_WITH_RABBITMQ_C])

  GATBPS_FINISH_WYNA(
    [--with-kestrel-rabbitmq],
    [WYNA_KESTREL_RABBITMQ])

  GATBPS_CHECK_EXPR(
    [whether to build the Kestrel RabbitMQ TA2 plugin],
    [WITH_KESTREL_RABBITMQ],
    [1
      && !WYNA_KESTREL_RABBITMQ_IS_NO
      && WITH_RABBITMQ_C
    ])

  [

    x=
    x=$][{x?}$][{WYNA_KESTREL_RABBITMQ_IS_YES?}
    x=$][{x?}$][{WITH_KESTREL_RABBITMQ?}
    case $][{x?} in 10)
      ]GATBPS_BARF([
        The Kestrel RabbitMQ TA2 plugin cannot be built.
      ])[
    esac

  ]

}])
