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

case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
x=`dirname "$x"` || exit $?
case $x in /) x= ;; esac
set -e || exit $?
. "$x"/../../sst.bash

sst_install_utility \
  git \
;

f=$(sst_find_dist_archive | sst_csf)
sst_csf f
cd "$tmpdir"
sst_extract_archive "$rundir/$f"
mv * foo
cd foo
git init
git add .
git commit -m .
cd ..
git clone foo bar
cd bar
install/on-$ON_X
