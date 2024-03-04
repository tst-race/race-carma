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

sst_expect_at_most_one_call() {

  local i
  local v
  local x

  # We need to look upward by two during the lazily loaded call.
  if [[ "${FUNCNAME[1]}" == "$FUNCNAME" ]]; then
    i=2
  else
    i=1
  fi
  readonly i

  if [[ "${FUNCNAME[i]}" == main ]]; then
    sst_barf '%s must only be called within a function' "$FUNCNAME"
  fi

  sst_expect_argument_count $# 0-1

  if (($# < 1)); then
    v=$(sst_underscore_slug "${FUNCNAME[i]}")_has_already_been_called
  else
    sst_expect_basic_identifier "$1"
    v=$1
  fi
  readonly v

  eval "x=\${$v+x}"
  if [[ "$x" ]]; then
    sst_barf '%s has already been called' "${FUNCNAME[i]}"
  fi

  eval "readonly $v=x"

}; readonly -f sst_expect_at_most_one_call
