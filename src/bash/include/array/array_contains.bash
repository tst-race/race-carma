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

#
# array_contains <result> <array> <value>
#
# If <array> is unset, it will be considered to be an empty array
# instead of producing an unbound variable error.
#

array_contains() {

  #
  # Before Bash 4.4, "${x[@]}" causes an error when x is an empty array
  # and set -u is enabled. The workaround is to write ${x[@]+"${x[@]}"}
  # instead. See <https://stackoverflow.com/q/7577052>.
  #

  if (($# == 3)); then
    sst_expect_basic_identifier "$1"
    sst_expect_basic_identifier "$2"
    eval '
      local r'$1$2'=0
      local x'$1$2'
      for x'$1$2' in ${'$2'[@]+"${'$2'[@]}"}; do
        if [[ "$x'$1$2'" == "$3" ]]; then
          r'$1$2'=1
          break
        fi
      done
      '$1'=$r'$1$2'
    '
  else
    sst_expect_argument_count $# 3
  fi

}; readonly -f array_contains
