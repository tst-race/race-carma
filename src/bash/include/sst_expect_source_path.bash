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

sst_expect_source_path() {

  local e
  local x

  for x; do

    e=

    case $x in

      "")
        e='Source paths must not be empty'
      ;;

      *[!-+./0-9A-Z_a-z]*)
        e='Source paths must only contain [-+./0-9A-Z_a-z] characters'
      ;;

      /*)
        e='Source paths must not begin with a / character'
      ;;

      */)
        e='Source paths must not end with a / character'
      ;;

      *//*)
        e='Source paths must not contain repeated / characters'
      ;;

      . | ./* | */./* | */.)
        e='Source paths must not contain any . components'
      ;;

      .. | ../* | */../* | */..)
        e='Source paths must not contain any .. components'
      ;;

      -* | */-*)
        e='Source path components must not begin with a - character'
      ;;

    esac

    if [[ "$e" != "" ]]; then
      x=$(sst_json_quote "$x")
      sst_barf "$e: $x"
    fi

  done

}; readonly -f sst_expect_source_path
