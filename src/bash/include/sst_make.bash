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

sst_make() {

  # Bash >=4.2: declare -g    sst_root_tmpdir

  declare    s
  declare    stderr_cache
  declare    x

  stderr_cache=$sst_root_tmpdir/$FUNCNAME.$BASHPID.stderr_cache
  readonly stderr_cache

  while :; do

    make "$@" 2>"$stderr_cache" && :
    s=$?

    cat "$stderr_cache" >&2 || sst_err_trap "$sst_last_command"

    if ((s != 0)); then

      # Retry if GNU Make fails with the "jobserver tokens" error. At
      # the time of writing this comment, 2022-11-23, it is not known
      # what causes this error.
      x=$(
        sed -n '
          /INTERNAL: Exiting with .* jobserver token.* available; should be .*!/ {
            p
            q
          }
        ' "$stderr_cache"
      ) || sst_err_trap "$sst_last_command"
      if [[ "$x" ]]; then
        sst_warn \
          "GNU Make failed with the \"jobserver tokens\" error." \
          "Retrying." \
        || :
        continue
      fi

    fi

    return $s

  done

}; readonly -f sst_make
