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

sst_am_finish() {

  # Bash >=4.2: declare -g -a sst_am_distribute_vars

  case ${sst_am_start_has_been_called+x} in
    "")
      sst_barf 'sst_am_start has not been called'
    ;;
  esac

  case ${sst_am_finish_has_been_called+x} in
    ?*)
      sst_barf 'sst_am_finish has already been called'
    ;;
  esac
  sst_am_finish_has_been_called=x
  readonly sst_am_finish_has_been_called

  case $# in
    0)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  #---------------------------------------------------------------------

  if [[ "${sst_am_distribute_vars[@]+x}" ]]; then
    printf '%s' "${sst_am_distribute_vars[@]}" >>"$autogen_am_file"
  fi

  #---------------------------------------------------------------------

}; readonly -f sst_am_finish
