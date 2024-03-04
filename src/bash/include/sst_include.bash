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

sst_include() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  declare    array_flag
  declare    declarations
  declare    definitions
  declare    file
  declare    includes
  declare    variable

  includes=
  declarations=
  definitions=
  for file; do
    includes+=" . $(sst_quote "$file");"
    file=${file##*/}
    if [[ "$file" == define_* ]]; then
      variable=${file#define_}
      variable=${variable%%.*}
      if [[ "$variable" == *-[Aa] ]]; then
        array_flag=" -${variable##*-}"
        variable=${variable%-*}
      else
        array_flag=
      fi
      if ((!SST_NDEBUG)); then
        sst_expect_basic_identifier "$variable"
      fi
      declarations+=" declare$array_flag $variable;"
      declarations+=" declare ${variable}_is_set;"
      definitions+=" define_$variable;"
    fi
  done
  readonly includes
  readonly declarations
  readonly definitions

  printf '%s\n' "$includes$declarations$definitions"

}; readonly -f sst_include
