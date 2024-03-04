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

sst_array_cmp() {

  local -r sst_ir='^(0|[1-9][0-9]*)$'

  local sst_xs
  local sst_ys
  local sst_xis
  local sst_yis
  local sst_xs_size
  local sst_ys_size
  local sst_min
  local sst_i
  local sst_xi
  local sst_yi
  local sst_x
  local sst_y

  sst_expect_argument_count $# 2

  sst_xs=$1
  sst_ys=$2

  sst_expect_basic_identifier "$sst_xs"
  sst_expect_basic_identifier "$sst_ys"

  eval sst_xis="(\${$sst_xs[@]+\"\${!$sst_xs[@]}\"})"
  eval sst_yis="(\${$sst_ys[@]+\"\${!$sst_ys[@]}\"})"

  eval sst_xs_size="\${$sst_xs[@]+\${#$sst_xs[@]}}"
  eval sst_ys_size="\${$sst_ys[@]+\${#$sst_ys[@]}}"

  if ((sst_xs_size < sst_ys_size)); then
    sst_min=$sst_xs_size
  else
    sst_min=$sst_ys_size
  fi

  for ((sst_i = 0; sst_i != sst_min; ++sst_i)); do
    sst_xi=${sst_xis[sst_i]}
    sst_yi=${sst_yis[sst_i]}
    if [[ "$sst_xi" =~ $sst_ir && "$sst_yi" =~ $sst_ir ]]; then
      ((sst_xi < sst_yi)) && printf '%s\n' -1 && return
      ((sst_xi > sst_yi)) && printf '%s\n'  1 && return
    else
      [[ "$sst_xi" < "$sst_yi" ]] && printf '%s\n' -1 && return
      [[ "$sst_xi" > "$sst_yi" ]] && printf '%s\n'  1 && return
    fi
  done
  ((sst_xs_size < sst_ys_size)) && printf '%s\n' -1 && return
  ((sst_xs_size > sst_ys_size)) && printf '%s\n'  1 && return
  for ((sst_i = 0; sst_i != sst_min; ++sst_i)); do
    sst_xi=${sst_xis[sst_i]}
    sst_yi=${sst_yis[sst_i]}
    eval sst_x="\${$sst_xs[\$sst_xi]}"
    eval sst_y="\${$sst_ys[\$sst_yi]}"
    [[ "$sst_x" < "$sst_y" ]] && printf '%s\n' -1 && return
    [[ "$sst_x" > "$sst_y" ]] && printf '%s\n'  1 && return
  done
  printf '%s\n'  0

}; readonly -f sst_array_cmp
