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

sst_am_else() {

  # Bash >=4.2: declare -g -a sst_am_if_vars
  # Bash >=4.2: declare -g    sst_am_suspend_if_depth

  declare    a
  declare    b
  declare    n

  if ((SST_DEBUG)); then

    if (($# == 1)); then
      sst_expect_basic_identifier "${1#!}"
    elif (($# != 0)); then
      sst_expect_argument_count $# 0-1
    fi

    if ((${sst_am_suspend_if_depth=0} > 0)); then
      sst_barf \
        "$FUNCNAME must not be called" \
        "while sst_am_suspend_if is active." \
      ;
    fi

    if [[ ! "${sst_am_if_vars[@]+x}" ]]; then
      sst_barf "Orphan $FUNCNAME."
    fi

  fi

  n=${#sst_am_if_vars[@]}
  readonly n

  a=${sst_am_if_vars[n - 1]% *}
  b=${sst_am_if_vars[n - 1]##* }
  if [[ "$b" == -* ]]; then
    sst_barf "Orphan sst_am_else."
  fi
  b=!$b
  b=${b#!!}
  readonly a
  readonly b

  if (($# == 1)); then
    if [[ "$1" != "$b" ]]; then
      sst_barf "Expected sst_am_else $b."
    fi
  fi

  sst_am_append <<<"else $b"

  sst_am_if_vars[n - 1]="$a -$b"

}; readonly -f sst_am_else
