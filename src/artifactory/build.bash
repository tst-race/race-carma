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
. "$x"/../../src/bash/sst.bash

#-----------------------------------------------------------------------

sst_expect_source_path "$BASH_SOURCE"
d=$(sst_get_prefix $BASH_SOURCE)
readonly d

dist_archive=$(sst_find_dist_archive)
readonly dist_archive

cd "$sst_tmpdir"
sst_extract_archive "$sst_rundir"/"$dist_archive"
cd *

./configure
make dist
make -j ${d%/}
cp ${d}kestrel-*-artifactory.tar.xz "$sst_rundir"/${d}.
