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

sst_get_variables() {

  declare    count
  declare    name
  declare -a names
  declare    regex

  names=$(set -o posix; set)
  names=${names//[] ()[]/}
  eval names="($names)"
  readonly names

  count=0
  for name in ${names[@]+"${names[@]}"}; do
    name=${name%%=*}
    if [[ "$name" == *$'\n'* ]]; then
      continue
    fi
    for regex; do
      if [[ "$name" =~ $regex ]]; then
        printf '%s\n' "$name"
        ((++count))
        continue 2
      fi
    done
  done
  readonly count

  if ((count == 0)); then
    echo
  fi

}; readonly -f sst_get_variables
