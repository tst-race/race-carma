dnl
dnl This file is from the CARMA package.
dnl
dnl The following copyright notice is generally applicable:
dnl
dnl      Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl The full copyright information depends on the distribution
dnl of the package. For more information, see the COPYING file.
dnl However, depending on the context in which you are viewing
dnl this file, the COPYING file may not be available.
dnl
AC_DEFUN_ONCE([DEFINE_CC], [[{

#
# The block that contains this comment is the expansion of the
# DEFINE_CC macro.
#]dnl
m4_ifdef(
  [DEFINE_CC_HAS_BEEN_CALLED],
  [gatbps_fatal([
    DEFINE_CC has already been called
  ])],
  [m4_define([DEFINE_CC_HAS_BEEN_CALLED])])[]dnl
m4_if(
  m4_eval([$# != 0]),
  [1],
  [gatbps_fatal([
    DEFINE_CC requires exactly 0 arguments
    ($# ]m4_if([$#], [1], [[was]], [[were]])[ given)
  ])])[]dnl
[

]AC_REQUIRE([GATBPS_PROG_CC])[

:;}]])[]dnl
