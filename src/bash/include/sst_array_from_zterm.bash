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

sst_array_from_zterm_helper() {

  od -A n -t o1 -v \
  | tr ' ' '\\' \
  | sed '
      1 s/^/$'\''/
      $ s/\\000$//
      $ s/$/'\''n/
      s/\\000/'\''n$'\''/g
    ' \
  | tr -d '\n' \
  | tr n '\n' \
  ;

  printf '%s\n' ")"

}; readonly -f sst_array_from_zterm_helper

sst_array_from_zterm() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  if ((!SST_NDEBUG)); then
    if (($# < 1)); then
      sst_expect_argument_count $# 1-
    fi
    sst_expect_basic_identifier "$1"
  fi

  printf '%s\n' "$1=(" >"$sst_root_tmpdir/$FUNCNAME.$$.x"

  if (($# == 1)); then
    sst_array_from_zterm_helper >>"$sst_root_tmpdir/$FUNCNAME.$$.x"
  else
    shift
    "$@" | sst_array_from_zterm_helper >>"$sst_root_tmpdir/$FUNCNAME.$$.x"
  fi

  . "$sst_root_tmpdir/$FUNCNAME.$$.x"

}; readonly -f sst_array_from_zterm
