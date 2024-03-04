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
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and run them
# with "command", and we need to explicitly call sst_err_trap on error
# to handle errexit suspension correctly. errexit suspension will occur
# when the user uses idioms such as "foo || s=$?" or "if foo; then" and
# foo triggers our automatic utility installation system. In this case,
# we want to maintain the behavior expected by the user but still barf
# if the installation of foo fails.
#

sst_push_var() {

  sst_expect_argument_count $# 1-2

  sst_expect_basic_identifier "$1"

  eval '
    # Bash >=4.2: declare -g sst_var_depth_'$1'
    local -r sst_d=${sst_var_depth_'$1'-0}
    sst_var_depth_'$1'=$((sst_d + 1))

    # Bash >=4.2: declare -g sst_var_unset_${sst_d}_'$1'
    # Bash >=4.2: declare -g sst_var_value_${sst_d}_'$1'

    if [[ "${'$1'+x}" ]]; then
      eval sst_var_unset_${sst_d}_'$1'=
      eval sst_var_value_${sst_d}_'$1'=\$'$1'
    else
      eval sst_var_unset_${sst_d}_'$1'=1
    fi

    if (($# == 1)); then
      unset '$1'
    else
      '$1'=$2
    fi
  '

}; readonly -f sst_push_var
