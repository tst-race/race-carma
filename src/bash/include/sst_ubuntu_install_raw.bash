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

sst_ubuntu_install_raw() {

  # Bash >=4.2: declare -g -A sst_ubuntu_install_raw_seen
  # Bash >=4.2: declare -g    sst_ubuntu_install_raw_setup

  declare    apt_get
  declare    cmd
  declare    package
  declare    package_q
  declare -A packages
  declare    r
  declare    s
  declare    stderr_cache
  declare    x

  stderr_cache=$sst_root_tmpdir/$FUNCNAME.$BASHPID.stderr_cache
  readonly stderr_cache

  packages=()

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_ubuntu_install_raw_seen[$package]-}" ]]; then
      continue
    fi
    sst_ubuntu_install_raw_seen[$package]=1

    package_q=$(sst_smart_quote "$package") || sst_barf

    # Skip this package if it's already installed.
    cmd="dpkg-query -W -f '\${db:Status-Status}' -- $package_q"
    r=$(eval "$cmd" 2>"$stderr_cache") && s=0 || s=$?
    if [[ $s == 0 && "$r" == installed ]]; then
      continue
    fi
    if [[ $s != 0 && $s != 1 ]]; then
      cat <"$stderr_cache" >&2 || :
      sst_barf "Command failed with exit status $s: $cmd"
    fi

    packages[$package_q]=1

  done

  if ((${#packages[@]} == 0)); then
    return
  fi

  #
  # Note that DPkg::Lock::Timeout requires apt-get >=1.9.11, but it's
  # fine to set it in older versions too, as they just ignore it.
  #

  apt_get="apt-get"
  apt_get+=" -o DPkg::Lock::Timeout=-1"
  apt_get+=" -q"
  if [[ ! -t 0 ]]; then
    apt_get+=" -y"
    apt_get="DEBIAN_FRONTEND=noninteractive $apt_get"
  fi

  if [[ ! "${sst_ubuntu_install_raw_setup-}" ]]; then
    sst_ubuntu_install_raw_setup=1
    readonly sst_ubuntu_install_raw_setup
    x=$(sst_type -f -p sudo)
    if [[ "$x" ]]; then
      cmd="sudo $apt_get update"
    else
      cmd="su -c '$apt_get update && $apt_get install sudo'"
    fi
    sst_warn "Running command: ($cmd)."
    sst_dpkg_loop "$cmd >&2"
  fi

  unset 'packages[sudo]'

  readonly packages

  if ((${#packages[@]} == 0)); then
    return
  fi

  apt_get="sudo $apt_get"
  readonly apt_get

  cmd="$apt_get install ${!packages[@]}"
  sst_warn "Running command: ($cmd)."
  sst_dpkg_loop "$cmd >&2"

}; readonly -f sst_ubuntu_install_raw
