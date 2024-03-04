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

jq_expect_type() {

  case $# in
    3)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  case $3 in
    array | boolean | null | number | object | string)
    ;;
    *)
      sst_barf '$3: invalid type: %s' "$3"
    ;;
  esac

  sst_jq_expect "
    $1 | type == \"$3\"
  " '%s: %s: expected %s' "$2" "$1" $3 <"$2"

}; readonly -f jq_expect_type
