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

sst_am_suspend_if() {

  # Bash >=4.2: declare -g -a sst_am_if_vars
  # Bash >=4.2: declare -g    sst_am_suspend_if_depth
  # Bash >=4.2: declare -g -a sst_am_suspend_if_vars

  declare    i
  declare    n

  if ((SST_DEBUG)); then
    if (($# > 0)); then
      sst_expect_argument_count $# 0
    fi
  fi

  if ((${sst_am_suspend_if_depth=0} == 0)); then
    if [[ "${sst_am_if_vars[@]+x}" ]]; then
      n=${#sst_am_if_vars[@]}
    else
      n=0
    fi
    readonly n
    if ((n == 0)); then
      sst_am_suspend_if_vars=()
    else
      sst_am_suspend_if_vars=("${sst_am_if_vars[@]}")
      for ((i = 0; i < n; ++i)); do
        sst_am_endif
      done
    fi
  fi
  sst_am_suspend_if_depth=$((sst_am_suspend_if_depth + 1))

}; readonly -f sst_am_suspend_if
