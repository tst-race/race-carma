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

sst_regex_escape_list() {

  declare    output

  if (($# == 0)); then
    output='($.)'
  else
    output='('$(sst_regex_escape "$1")
    while shift && (($# > 0)); do
      output+='|'$(sst_regex_escape "$1")
    done
    output+=')'
  fi
  readonly output

  printf '%s\n' "$output"

}; readonly -f sst_regex_escape_list
