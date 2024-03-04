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

sst_expect_ag_json_helper() {

  declare    handler
  declare    json
  declare    p_target
  declare    x

  sst_expect_not_subshell sst_expect_ag_json

  sst_expect_argument_count $# 2-3

  p_target=$1
  readonly p_target
  if [[ "$p_target" != - ]]; then
    sst_expect_basic_identifier "$p_target"
  fi

  json=$2
  readonly json

  if (($# == 2)); then
    # The first call will always come in through the automatic function
    # loader, in which case the calling function is actually two frames
    # upward, not one. Plus one since we're in a helper function.
    if [[ "${FUNCNAME[2]}" == "${FUNCNAME[1]}" ]]; then
      handler=${FUNCNAME[3]}
    else
      handler=${FUNCNAME[2]}
    fi
  else
    handler=$3
  fi
  readonly handler
  sst_expect_basic_identifier "$handler"

  sst_expect_source_path "$json"
  sst_expect_extension $json .ag.json

  jq_expect_object . $json

  sst_jq_get_string $json .handler x
  if [[ "$x" != $handler ]]; then
    x=$(sst_json_quote "$x")
    sst_barf "$json: .handler: expected \"$handler\" but got $x"
  fi

  sst_am_distribute $json

  printf '%s\n' ${json%.ag.json}

}; readonly -f sst_expect_ag_json_helper

sst_expect_ag_json() {

  sst_expect_ag_json_helper "$@" >"$sst_root_tmpdir/$FUNCNAME.target"
  if [[ "$1" != - ]]; then
    eval $1='$(cat "$sst_root_tmpdir/$FUNCNAME.target")'
  fi

}; readonly -f sst_expect_ag_json
