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

sst_am_var_set() {

  # Bash >=4.2: declare -g    SST_NDEBUG
  # Bash >=4.2: declare -g -A sst_am_var_const
  # Bash >=4.2: declare -g -A sst_am_var_value

  declare    emit
  declare    first
  declare    value
  declare    var

  if ((!SST_NDEBUG)); then
    sst_expect_not_subshell
    if (($# == 0)); then
      sst_expect_argument_count $# 1-
    fi
  fi

  var=$1
  readonly var

  if ((!SST_NDEBUG)); then
    sst_expect_basic_identifier "$var"
  fi

  if [[ "${sst_am_var_value[$var]+x}" ]]; then
    first=0
  else
    first=1
  fi
  readonly first

  if ((first)); then
    sst_am_var_const[$var]=0
  elif ((${sst_am_var_const[$var]})); then
    sst_barf \
      "The Automake variable $var is const and cannot be modified." \
    ;
  fi

  shift

  value="$@"
  value=${value#"${value%%[![:blank:]]*}"}
  value=${value%"${value##*[![:blank:]]}"}
  readonly value

  emit=1
  if ((first)) && [[ ! "$value" ]]; then
    # GATBPS already initializes these to be empty.
    case $var in - \
    | EXTRA_DIST \
    | SUFFIXES \
    | include_HEADERS \
    | nodist_include_HEADERS \
    )
      emit=0
    esac
  fi
  readonly emit

  if ((emit)); then
    sst_am_append <<<"$var =${value:+ $value}"
  fi

  sst_am_var_value[$var]=$value

}; readonly -f sst_am_var_set
