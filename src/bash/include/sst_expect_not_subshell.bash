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

sst_expect_not_subshell() {

  # Bash >=4.2: declare -g    SST_DEBUG

  declare    f

  if ((SST_DEBUG)); then
    if (($# == 1)); then
      sst_expect_basic_identifier "$1"
    elif (($# != 0)); then
      sst_expect_argument_count $# 0-1
    fi
  fi

  if ((BASH_SUBSHELL > 0)); then

    if (($# == 1)); then
      f=$1
    elif [[ "${FUNCNAME[1]}" == "${FUNCNAME[0]}" ]]; then
      f=${FUNCNAME[2]}
    else
      f=${FUNCNAME[1]}
    fi
    readonly f

    if [[ "$f" == - ]]; then
      sst_barf "This code must not be run in a subshell."
    else
      sst_barf "The $f function must not be called in a subshell."
    fi

  fi

}; readonly -f sst_expect_not_subshell
