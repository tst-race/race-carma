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

sst_jq_get_string_helper() {

  local filter
  local json
  local script
  local var

  json=$1
  readonly json

  filter=$2
  readonly filter

  var=$3
  readonly var

  printf '
    %s=
    %s_error=()
  ' "$var" "$var"

  script='
    .[0]
    | ('"$filter"')
    | if . == null then
        "'$var'=" + ("" | @sh)
      elif type == "string" then
        "'$var'=" + (. | @sh)
      else
        "'$var'_error+=(\": value must be a string\")"
      end
  '
  readonly script

  if [[ "$json" == - ]]; then
    jq -r -s " $script"
  elif [[ "$json" == [/A-Za-z0-9._-]* ]]; then
    jq -r -s " $script" <"$json"
  else
    jq -r -s " $script" <<<"$json"
  fi

}; readonly -f sst_jq_get_string_helper

sst_jq_get_string() {

  sst_expect_argument_count $# 2-3

  if (($# == 2)); then
    set x "$1" ."$2" "$2"
    shift
  fi

  sst_expect_basic_identifier "$3"

  local ${3}_code
  local ${3}_error
  eval '
    '$3'_code=$(sst_jq_get_string_helper "$@")
    eval " $'$3'_code"
  '
  readonly ${3}_code
  readonly ${3}_error

  local display
  local error
  local filter
  local json

  json=$1
  readonly json

  filter=$2
  readonly filter

  eval 'error=${'$3'_error-}'
  readonly error

  if [[ "$error" != "" ]]; then
    if [[ "$json" == - ]]; then
      display='<stdin>'
    elif [[ "$json" == [/A-Za-z0-9._-]* ]]; then
      display=$(sst_smart_quote "$json")
    else
      display=$(sst_quote "$json")
    fi
    readonly display
    sst_barf "jq: $display: $filter$error"
  fi

}; readonly -f sst_jq_get_string
