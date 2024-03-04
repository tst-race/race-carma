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
# sst_am_start [<file>]
#
# Start constructing an accumulative Automake source file in which to
# collect autogen output. <file> should specify a file that lives either
# in or below the current directory. If <file> is not given, it defaults
# to autogen.am.
#

sst_am_start() {

  # Bash >=4.2: declare -g    autogen_am_file
  # Bash >=4.2: declare -g    sst_am_start_has_been_called

  if [[ "${sst_am_start_has_been_called-}" ]]; then
    sst_barf "sst_am_start has already been called."
  fi
  readonly sst_am_start_has_been_called=1

  if (($# == 0)); then
    autogen_am_file=autogen.am
  elif (($# == 1)); then
    autogen_am_file=$1
  else
    sst_expect_argument_count $# 0-1
  fi

  autogen_am_file=$(sst_add_slash_abs_prefix "$autogen_am_file")
  autogen_am_file=${autogen_am_file%/}
  readonly autogen_am_file

  autogen_print_am_header >"$autogen_am_file"

  sst_trap_append '
    case $sst_trap_entry_status in
      0)
        case ${sst_am_finish_has_been_called+x} in
          "")
            sst_barf "you forgot to call sst_am_finish"
          ;;
        esac
      ;;
    esac
  ' EXIT

  sst_am_var_add EXTRA_DIST autogen

}; readonly -f sst_am_start
