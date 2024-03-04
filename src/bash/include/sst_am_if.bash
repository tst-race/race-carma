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

sst_am_if() {

  # Bash >=4.2: declare -g -a sst_am_if_vars
  # Bash >=4.2: declare -g    sst_am_suspend_if_depth

  declare    n

  if ((SST_DEBUG)); then

    if (($# != 1)); then
      sst_expect_argument_count $# 1
    fi
    sst_expect_basic_identifier "${1#!}"

    if ((${sst_am_suspend_if_depth=0} > 0)); then
      sst_barf \
        "$FUNCNAME must not be called" \
        "while sst_am_suspend_if is active." \
      ;
    fi

  fi

  if [[ "${sst_am_if_vars[@]+x}" ]]; then
    n=${#sst_am_if_vars[@]}
  else
    n=0
  fi
  readonly n

  sst_am_if_vars[n]=" $1"

  sst_am_append <<<"if $1"

}; readonly -f sst_am_if
