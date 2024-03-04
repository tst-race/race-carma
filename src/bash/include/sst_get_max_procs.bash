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

sst_get_max_procs() {

  # Bash >=4.2: declare -g sst_max_procs

  declare    n

  if (($# > 0)); then
    sst_expect_argument_count $# 0
  fi

  if [[ "${sst_max_procs+x}" ]]; then
    return
  fi

  sst_max_procs=1

  if [[ -f /proc/cpuinfo ]]; then
    n=$(
      awk '
        BEGIN {
          n = 0;
        }
        /^processor[\t ]*:/ {
          ++n;
        }
        END {
          print n;
        }
      ' /proc/cpuinfo
    )
    if ((n > 0)); then
      sst_max_procs=$n
    fi
  fi

  readonly sst_max_procs

}; readonly -f sst_get_max_procs
