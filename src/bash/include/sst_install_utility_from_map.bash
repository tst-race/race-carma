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

sst_install_utility_from_map() {

  # Bash >=4.2: declare -g sst_install_utility_from_map_initialized
  # Bash >=4.2: declare -g -A sst_utility_prefixes
  # Bash >=4.2: declare -g -A sst_utility_programs
  # Bash >=4.2: declare -g -A sst_utility_suffixes

  local info
  local map
  local -A packages
  local s
  local utility
  local x
  local y

  sst_expect_argument_count $# 1-

  map=$1
  readonly map
  sst_expect_basic_identifier "$map"

  shift

  sst_get_distro || sst_barf
  sst_get_distro_version || sst_barf

  packages=()

  for utility; do

    if [[ "${sst_utility_programs[$utility]-}" ]]; then
      continue
    fi

    info="$utility $sst_distro_version"
    eval 'info=${'$map'[$info]-}'

    if [[ ! "$info" ]]; then
      x=$(sst_smart_quote "$utility") || x=$utility
      sst_barf \
        "Missing installation information for" \
        "$x on $sst_distro $sst_distro_version." \
      ;
    fi

    if [[ "$info" == *' '* ]]; then
      x=${info%% *}
      sst_utility_prefixes[$utility]=
      sst_utility_programs[$utility]=$x
      sst_utility_suffixes[$utility]=
      type -f "$x" &>/dev/null && s=0 || s=$?
      if ((s == 1)); then
        sst_push_var IFS ' '
        for x in ${info#* }; do
          packages[$x]=1
        done
        sst_pop_var IFS
      elif ((s != 0)); then
        y=$(sst_smart_quote "$x") || y=$x
        sst_barf \
          "The following command failed with exit status $s:" \
          "type -f $y &>/dev/null" \
        ;
      fi
    else
      sst_expect_basic_identifier "$info"
      $info || sst_barf
    fi

  done

  sst_${sst_distro}_install_raw "${!packages[@]}" || sst_barf

}; readonly -f sst_install_utility_from_map
