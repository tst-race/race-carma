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
# autogen_am_var_append <name> [<text>...]
#

autogen_am_var_append() {

  local name
  local text

  if (( $# == 0 )); then
    sst_barf 'invalid argument count: %d' $#
  fi

  name=$1
  readonly name
  shift
  sst_expect_basic_identifier "$name"

  for text; do
    sst_am_append <<EOF
$name += $text
EOF
  done

}; readonly -f autogen_am_var_append
