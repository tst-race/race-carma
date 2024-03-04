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
# Parse the post_commit_script arguments
#-----------------------------------------------------------------------

sst_expect_argument_count $# 2
readonly dist_dir="$1"
readonly repo_dir="$2"

#-----------------------------------------------------------------------

RACE_VERSION=$(cat build-aux/RACE_VERSION)
readonly RACE_VERSION

ARTIFACTORY_REVISION=$(cat build-aux/ARTIFACTORY_REVISION)
readonly ARTIFACTORY_REVISION

tag=race-$RACE_VERSION-carma-r$ARTIFACTORY_REVISION
readonly tag

git tag ${GPG_SECRET_KEY:+-s} -m "$tag" "$tag"
