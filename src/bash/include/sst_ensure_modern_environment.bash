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

sst_ensure_modern_environment() {

  # Bash >=4.2: declare -g    SST_MODERN_ENVIRONMENT
  # Bash >=4.2: declare -g    sst_distro

  declare    cmd
  declare    x

  if [[ ! "${SST_MODERN_ENVIRONMENT+x}" ]]; then
    sst_get_distro
    case $sst_distro in '')
    ;; centos)
      sst_get_distro_version
      case $sst_distro_version in '')

      #-----------------------------------------------------------------
      # centos 7
      #-----------------------------------------------------------------
      ;; 7)

        sst_centos_install_raw centos-release-scl
        sst_centos_install_raw devtoolset-11
        cmd=
        for x; do
          cmd+=${cmd:+ }$(sst_quote "$x")
        done
        export SST_MODERN_ENVIRONMENT=
        sst_exec scl enable devtoolset-11 "$cmd"

      #-----------------------------------------------------------------
      esac
    esac
  fi

}; readonly -f sst_ensure_modern_environment
