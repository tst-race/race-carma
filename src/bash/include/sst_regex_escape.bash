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

sst_regex_escape() {

  declare    c
  declare    i
  declare    input
  declare    output

  if (($# == 0)); then
    input=$(cat | sst_csf)
    sst_csf input
  else
    input="$@"
  fi
  readonly input

  output=
  for ((i = 0; i < ${#input}; ++i)); do
    c=${input:i:1}
    if [[ "$c" == '\' ]]; then
      output+=[\\$c]
    else
      output+=[$c]
    fi
  done
  readonly output

  printf '%s\n' "$output"

}; readonly -f sst_regex_escape
