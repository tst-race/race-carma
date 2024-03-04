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
# Note that distributed files should generally never be conditional, as
# it does not make sense for the output of "make dist" to be dynamic. In
# other words, running "make dist" should always produce the same output
# regardless of how the configure script was run and regardless of what
# the configure script observed about the environment.
#
# sst_am_distribute handles this by storing the list of distributed
# files in memory, which is later written to the autogen.am file by
# sst_am_finish after all conditionals have ended. This means the files
# are distributed unconditionally, even if sst_am_distribute was called
# within an sst_am_if block.
#

sst_am_distribute() {

  # Bash >=4.2: declare -g    sst_am_distribute_i
  # Bash >=4.2: declare -g -A sst_am_distribute_seen
  # Bash >=4.2: declare -g -a sst_am_distribute_vars

  declare    i
  declare    n
  declare    path

  sst_expect_not_subshell

  # n should be hardcoded to an integer value in [1, k+1], where k is
  # the number from the highest numbered GATBPS_DISTFILES_k target in
  # GATBPS. Note that newer GATBPS versions will only ever increase k.
  n=100
  readonly n

  for path; do
    sst_expect_source_path "$path"
    if [[ ! -f $path && ! -d $path && -e $path ]]; then
      path=$(sst_quote "$path")
      sst_barf \
        "Path must either exist as a file," \
        "exist as a directory, or not exist: $path." \
      ;
    fi
    if [[ "${sst_am_distribute_seen["$path"]-}" ]]; then
      continue
    fi
    sst_am_distribute_seen["$path"]=1
    i=${sst_am_distribute_i-0}
    sst_am_distribute_i=$(((i + 1) % n))
    if [[ ! "${sst_am_distribute_vars[i]-}" ]]; then
      sst_am_distribute_vars[i]=
    fi
    sst_am_distribute_vars[i]+="GATBPS_DISTFILES_$i += $path"$'\n'
  done

}; readonly -f sst_am_distribute
