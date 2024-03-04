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

sst_am_restore_if() {

  # Bash >=4.2: declare -g    sst_am_suspend_if_depth
  # Bash >=4.2: declare -g -a sst_am_suspend_if_vars

  declare    a
  declare    b
  declare    i
  declare    n

  if ((SST_DEBUG)); then
    if (($# > 0)); then
      sst_expect_argument_count $# 0
    fi
    if ((${sst_am_suspend_if_depth=0} == 0)); then
      sst_barf "Orphan sst_am_restore_if."
    fi
  fi

  if ((--sst_am_suspend_if_depth == 0)); then
    n=${#sst_am_suspend_if_vars[@]}
    readonly n
    for ((i = 0; i < n; ++i)); do
      a=${sst_am_suspend_if_vars[i]}
      a="${a# } "
      b=${a#* }
      a=${a%% *}
      if [[ ${a:0:1} != - ]]; then
        sst_am_if $a
      else
        a=!${a#-}
        a=${a#!!}
        sst_am_if $a
        if [[ ! "$b" ]]; then
          sst_am_else
        else
          a=${b%% *}
          b=${b#* }
          while [[ "$b" ]]; do
            a=!${a#-}
            a=${a#!!}
            sst_am_elif $a
            a=${b%% *}
            b=${b#* }
          done
          if [[ ${a:0:1} != - ]]; then
            sst_am_elif $a
          else
            a=!${a#-}
            a=${a#!!}
            sst_am_elif $a
            sst_am_else
          fi
        fi
      fi
    done
  fi

}; readonly -f sst_am_restore_if
