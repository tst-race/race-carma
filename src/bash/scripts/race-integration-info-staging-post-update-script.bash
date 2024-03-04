#! /bin/sh -
#
# Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an "AS
# IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language
# governing permissions and limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

#-----------------------------------------------------------------------
# Load the prelude
#-----------------------------------------------------------------------

case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
x=`dirname "$x"` || exit $?
case $x in /) x= ;; esac
set -e || exit $?
. "$x"/../sst.bash

#-----------------------------------------------------------------------

f() {

  declare dist_dir
  declare g
  declare repo_dir
  declare -g sst_is0atty
  declare u

  sst_expect_argument_count $# 2

  dist_dir=$1
  readonly dist_dir

  repo_dir=$2
  readonly repo_dir

  git rm --cached '*'

  "$repo_dir"/build-aux/adock \
    -r asciidoctor-diagram \
    doc/manual/using_carma_with_race.adoc \
  ;

  u=$(id -u)
  g=$(id -g)
  docker run --rm -i ${sst_is0atty:+-t} \
    -u $u:$g \
    -v "$PWD":/x \
    selenium/standalone-chrome \
    /opt/google/chrome/chrome \
      --disable-gpu \
      --headless \
      --no-sandbox \
      --print-to-pdf-no-header \
      --print-to-pdf=/x/carma-race-integration-info.pdf \
      --run-all-compositor-stages-before-draw \
      --virtual-time-budget=30000 \
      /x/doc/manual/using_carma_with_race.html \
  ;

  git add carma-race-integration-info.pdf

  cp carma-race-integration-info.pdf "$repo_dir"

}; f "$@"
