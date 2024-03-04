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

sst_cygwin_install_raw() {

  # Bash >=4.2: declare -g -A sst_cygwin_install_raw_seen

  declare -a missing
  declare    package
  declare    x

  missing=()

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_cygwin_install_raw_seen[$package]-}" ]]; then
      continue
    fi
    sst_cygwin_install_raw_seen[$package]=x

    # Skip this package if it's already installed.
    x=$(
      cygcheck -c "$package" | sed -n '/OK$/ p'
    ) || sst_err_trap "$sst_last_command"
    if [[ "$x" ]]; then
      continue
    fi

    missing+=("$package")

  done

  readonly missing

  if ((${#missing[@]} > 0)); then
    x=$(
      sst_human_list : "${missing[@]}"
    ) || sst_err_trap "$sst_last_command"
    if ((${#missing[@]} > 1)); then
      x="Missing Cygwin packages: $x."
      x="$x Please install them using the Cygwin setup program."
    else
      x="Missing Cygwin package: $x."
      x="$x Please install it using the Cygwin setup program."
    fi
    sst_barf "$x"
  fi

}; readonly -f sst_cygwin_install_raw
