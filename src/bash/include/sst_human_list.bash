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

sst_human_list() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  declare -a adjust
  declare    output
  declare    x

  if ((!SST_NDEBUG)); then
    if (($# == 0)); then
      sst_expect_argument_count $# 1-
    fi
  fi

  adjust=()
  while (($# > 0)) && [[ "$1" != : ]]; do
    adjust+=("$1")
    shift
  done
  readonly adjust

  if ((!SST_NDEBUG)); then
    if (($# == 0)); then
      sst_barf "The : argument must always be given."
    fi
  fi

  shift

  if (($# == 0)); then
    output="none"
  elif (($# == 1)); then
    if ((${#adjust[@]} > 0)); then
      output=$("${adjust[@]}" "$1" | sst_csf)
      sst_csf output
    else
      output=$1
    fi
  elif (($# == 2)); then
    if ((${#adjust[@]} > 0)); then
      output=$("${adjust[@]}" "$1" | sst_csf)
      sst_csf output
      output+=' and '
      x=$("${adjust[@]}" "$2" | sst_csf)
      sst_csf x
      output+=$x
    else
      output="$1 and $2"
    fi
  else
    if ((${#adjust[@]} > 0)); then
      output=$("${adjust[@]}" "$1" | sst_csf)
      sst_csf output
    else
      output=$1
    fi
    shift
    while (($# > 1)); do
      output+=', '
      if ((${#adjust[@]} > 0)); then
        x=$("${adjust[@]}" "$1" | sst_csf)
        sst_csf x
        output+=$x
      else
        output+=$1
      fi
      shift
    done
    output+=', and '
    if ((${#adjust[@]} > 0)); then
      x=$("${adjust[@]}" "$1" | sst_csf)
      sst_csf x
      output+=$x
    else
      output+=$1
    fi
  fi
  readonly output

  printf '%s\n' "$output"

}; readonly -f sst_human_list
