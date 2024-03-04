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

dnl---------------------------------------------------------------------

AC_INIT(
  [[Kestrel]],
  m4_esyscmd_s([
    sh build-aux/gatbps-gen-version.sh
  ])m4_if(m4_sysval, 0, [], [
    m4_fatal([gatbps-gen-version.sh failed.])
  ]),
  [[kestrel@stealthsoftwareinc.com]],
  [[kestrel]])

dnl---------------------------------------------------------------------

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([build-aux/carma.srcdir])

dnl---------------------------------------------------------------------

m4_include([build-aux/gatbps.ac])

dnl---------------------------------------------------------------------
dnl Program checks
dnl---------------------------------------------------------------------

GATBPS_PROG_ASCIIDOCTOR
GATBPS_PROG_AWK
GATBPS_PROG_DOCKER
GATBPS_PROG_GIT
GATBPS_PROG_GREP
GATBPS_PROG_LN_S
GATBPS_PROG_MKDIR_P
GATBPS_PROG_OPENSSL
GATBPS_PROG_SED
GATBPS_PROG_WGET

dnl---------------------------------------------------------------------

[

]gatbps_check_macros[ || exit

]GATBPS_DEFINE_VERSION(
  [AC_],
  [PACKAGE_VERSION])[

]

[ARTIFACTORY_REVISION]='m4_esyscmd_s([sh build-aux/ARTIFACTORY_REVISION.sh])'
m4_if(m4_sysval, [0], [], [m4_fatal([ARTIFACTORY_REVISION.sh failed])])
AC_SUBST([ARTIFACTORY_REVISION])

[RACE_VERSION]='m4_esyscmd_s([sh build-aux/RACE_VERSION.sh])'
m4_if(m4_sysval, [0], [], [m4_fatal([RACE_VERSION.sh failed])])
AC_SUBST([RACE_VERSION])

[RIB_VERSION]='m4_esyscmd_s([sh build-aux/RIB_VERSION.sh])'
m4_if(m4_sysval, [0], [], [m4_fatal([RIB_VERSION.sh failed])])
AC_SUBST([RIB_VERSION])

[

]GATBPS_DEFINE_DATE(
  [AC_],
  [PACKAGE_DATE],
  m4_esyscmd_s([
    sh build-aux/gatbps-gen-date.sh
  ])m4_if(m4_sysval, 0, [], [
    m4_fatal([gatbps-gen-date.sh failed])
  ]))[

]AC_COPYRIGHT(
[[Some parts of this file are from the CARMA package.

The following copyright notice is generally applicable:

     Copyright (C) Stealth Software Technologies, Inc.

The full copyright information depends on the distribution
of the package. For more information, see the COPYING file.
However, depending on the context in which you are viewing
this file, the COPYING file may not be available.
]])[

]AM_INIT_AUTOMAKE([
  -Wall
  foreign
  nostdinc
  subdir-objects
  tar-ustar
])[

]

m4_include([build-aux/autogen.ac])

LT_INIT([
  disable-static
  shared
  win32-dll
])

[

]m4_include([src/misc/nlohmann/json.hpp.ac])[

]dnl end_includes
[

]

GATBPS_CONFIG_FILE([build-aux/INPUT_FILTER.sh])
GATBPS_CONFIG_FILE([build-aux/PACKAGE_STRING.df])
GATBPS_CONFIG_FILE([src/rib-tools/run-carma.bash])
GATBPS_CONFIG_FILE([src/rib-tools/run-stub.bash])

dnl---------------------------------------------------------------------
dnl Program checks
dnl---------------------------------------------------------------------

GATBPS_PROG_ASCIIDOCTOR

dnl---------------------------------------------------------------------

[

]GATBPS_CONFIG_FILE_SUBST[
]GATBPS_CONFIG_LATER_SUBST[
]GATBPS_CP_SUBST[
]GATBPS_GIT_ARCHIVE_SUBST[
]GATBPS_GIT_BUNDLE_SUBST[
]GATBPS_GIT_CLONE_SUBST[
]GATBPS_WGET_SUBST[

]AC_CONFIG_FILES([Makefile])[
]AC_CONFIG_HEADERS([src/misc/config.h])[

#
# Restore the normal Libtool behavior on systems that prefer to set
# `link_all_deplibs=no`, as this generally causes problems with our
# workflow. See <https://stackoverflow.com/q/11802727>.
#

link_all_deplibs=yes
link_all_deplibs_CXX=yes

#
#

]AC_OUTPUT[

]
