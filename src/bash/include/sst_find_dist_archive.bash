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

sst_find_dist_archive() {

  declare    archive
  declare -r dir="$sst_root_tmpdir/$FUNCNAME.$BASHPID"
  declare    n
  declare    x
  declare    y

  if ((SST_DEBUG)); then
    if (($# != 0)); then
      sst_expect_argument_count $# 0
    fi
  fi

  for archive in *.tar*; do

    rm -f -r "$dir"
    mkdir -p "$dir"/x

    sst_pushd "$dir"/x
    sst_extract_archive "$sst_rundir/$archive"
    sst_popd

    # The archive should have extracted to exactly one directory.
    n=0
    for x in "$dir"/x/*; do
      if ((++n > 1)); then
        continue 2
      elif [[ ! -d "$x" ]]; then
        continue 2
      fi
    done

    # The directory name should end with our version number.
    sst_install_utility awk git sed
    y=$(sh build-aux/gatbps-gen-version.sh)
    if [[ "$x" != *"$y" ]]; then
      continue
    fi

    # We found it.
    printf '%s\n' "$archive"
    return

  done

  sst_barf "Distribution archive not found."

}; readonly -f sst_find_dist_archive
