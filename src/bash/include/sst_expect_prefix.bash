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

sst_expect_prefix() {

  declare path
  declare prefix
  declare x

  if (($# == 0)); then
    path=$(cat | sst_csf)
    sst_csf path
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi
  readonly path

  prefix=$(sst_get_prefix "$path" | sst_csf)
  sst_csf prefix
  readonly prefix

  if [[ "$prefix" == "" ]]; then
    x=$(sst_smart_quote "$path")
    sst_barf "Path must have a nonempty prefix: $x"
  fi

  printf '%s\n' "$prefix"

}; readonly -f sst_expect_prefix
