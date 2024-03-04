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

sst_ubuntu_get_distro_version() {

  # Bash >=4.2: declare -g    SST_DEBUG

  if ((SST_DEBUG)); then
    if (($# != 0)); then
      sst_expect_argument_count $# 0
    fi
  fi

  if [[ -f /etc/os-release ]]; then
    sed -n '
      /^VERSION_ID=/ {
        s/^VERSION_ID=//
        s/"//g
        s/^16.04$/16.04/p
        s/^18.04$/18.04/p
        s/^20.04$/20.04/p
        s/^22.04$/22.04/p
        q
      }
    ' /etc/os-release
    return
  fi

}; readonly -f sst_ubuntu_get_distro_version
