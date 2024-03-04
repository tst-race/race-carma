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

sst_add_slash_abs_prefix() {

  declare    path

  if (($# == 0)); then
    path=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf path || sst_err_trap "$sst_last_command"
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi

  if [[ ! "$path" ]]; then
    sst_barf "The empty string is not a valid path."
  fi

  if [[ "$path" == *[!/] ]]; then
    path+=/
  fi

  if [[ "$path" ]]; then
    if [[ "$path" == .. || "$path" == */.. ]]; then
      path+=/.
    fi
    if [[ "$path" == */* ]]; then
      path=${path%/*}/
      if [[ "$path" != /* ]]; then
        if [[ "$PWD" == / ]]; then
          path=/$path
        else
          path=$PWD/$path
        fi
      fi
    elif [[ "$PWD" == / ]]; then
      path=/
    else
      path=$PWD/
    fi
  fi

  readonly path

  printf '%s\n' "$path" || sst_err_trap "$sst_last_command"

}; readonly -f sst_add_slash_abs_prefix
