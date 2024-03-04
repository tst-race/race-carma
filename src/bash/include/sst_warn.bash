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

sst_warn() {

  # Bash >=4.2: declare -g    CLICOLOR
  # Bash >=4.2: declare -g    CLICOLOR_FORCE

  declare    buffer
  declare    color_start
  declare    color_stop

  if [[ "${CLICOLOR_FORCE-0}" != 0 || \
        ( "${CLICOLOR-1}" != 0 && -t 2 ) ]]; then
    color_start=$'\x1B[33m'
    color_stop=$'\x1B[0m'
  else
    color_start=
    color_stop=
  fi
  readonly color_start
  readonly color_stop

  buffer="$color_start$0: Warning: $@$color_stop"
  readonly buffer

  printf '%s\n' "$buffer" >&2 || :

}; readonly -f sst_warn
