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

sst_dpkg_loop() {

  # Bash >=4.2: declare -g    sst_root_tmpdir

  declare    n
  declare    s
  declare    stderr_cache
  declare    x

  stderr_cache=$sst_root_tmpdir/$FUNCNAME.$BASHPID.stderr_cache
  readonly stderr_cache

  n=2
  while :; do
    # Note that putting 2>"$stderr_cache" inside the eval means that,
    # e.g., sst_dpkg_loop 'sudo apt-get update >&2' will forward its
    # original stdout to stderr and its original stderr to the cache
    # file. This is useful for running incidental dpkg commands that
    # shouldn't interfere with stdout.
    eval "$@" '2>"$stderr_cache"' && :
    s=$?
    if ((s == 0)); then
      break
    fi
    x=$(
      sed '
        /is another process using it/ {
          p
          q
        }
      ' "$stderr_cache"
    ) || sst_err_trap "$sst_last_command"
    if [[ ! "$x" ]]; then
      cat "$stderr_cache" >&2
      exit $s
    fi
    sst_warn \
      "Unable to acquire the dpkg lock." \
      "Retrying in $n seconds." \
    ;
    sleep $n || sst_err_trap "$sst_last_command"
    n=$((n + (n < 10)))
  done

}; readonly -f sst_dpkg_loop
