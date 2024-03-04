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

sst_array_contains() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  if ((!SST_NDEBUG)); then
    if (($# < 1)); then
      sst_expect_argument_count $# 1- || sst_err_trap "$sst_last_command"
    fi
    sst_expect_basic_identifier "$1" || sst_err_trap "$sst_last_command"
  fi

  eval '

    shift

    declare    x'$1'
    declare    y'$1'

    if (($# == 0)); then
      x'$1'=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
      sst_csf x'$1' || sst_err_trap "$sst_last_command"
    else
      x'$1'="$@"
    fi
    readonly x'$1'

    for y'$1' in ${'$1'[@]+"${'$1'[@]}"}; do
      if [[ "$y'$1'" == "$x'$1'" ]]; then
        return 0
      fi
    done

    return 1

  '

}; readonly -f sst_array_contains
