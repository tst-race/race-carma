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
# sst_trap_append <arg> <sigspec>...
#

sst_trap_append() {

  local arg
  local sigspec
  local x

  case $# in
    0 | 1)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  arg=$1
  readonly arg
  shift

  for sigspec; do

    x=$(trap -p -- "$sigspec")

    case $x in
      ?*)
        eval "set -- $x"
        shift $(($# - 2))
        x=$1$'\n'$arg
      ;;
      *)
        x=$arg
      ;;
    esac

    trap -- "$x" "$sigspec"

  done

}; readonly -f sst_trap_append
