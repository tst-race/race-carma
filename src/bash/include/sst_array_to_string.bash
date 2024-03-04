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

sst_array_to_string() {

  local -r sst_regex='^(0|[1-9][0-9]*)$'

  local sst_xs
  local sst_xis
  local sst_xs_size
  local sst_space
  local sst_i
  local sst_xi
  local sst_x

  sst_expect_argument_count $# 1

  sst_xs=$1

  sst_expect_basic_identifier "$sst_xs"

  eval sst_xis="(\${$sst_xs[@]+\"\${!$sst_xs[@]}\"})"

  eval sst_xs_size="\${$sst_xs[@]+\${#$sst_xs[@]}}"

  printf '('
  sst_space=
  for ((sst_i = 0; sst_i != sst_xs_size; ++sst_i)); do
    sst_xi=${sst_xis[sst_i]}
    eval sst_x="\${$sst_xs[\$sst_xi]}"
    if [[ ! "$sst_xi" =~ $sst_regex ]]; then
      sst_xi=$(sst_quote <<<"$sst_xi")
    fi
    sst_x=$(sst_quote <<<"$sst_x")
    printf '%s[%s]=%s' "$sst_space" "$sst_xi" "$sst_x"
    sst_space=' '
  done
  printf ')\n'

}; readonly -f sst_array_to_string
