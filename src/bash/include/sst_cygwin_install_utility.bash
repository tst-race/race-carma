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

sst_cygwin_install_utility() {

  # Bash >=4.2: declare -g -A sst_cygwin_install_utility_map
  # Bash >=4.2: declare -g sst_cygwin_install_utility_once

  if ((!${sst_cygwin_install_utility_once-0})); then

    sst_cygwin_install_utility_once=1

    sst_cygwin_install_utility_map=(

      ['awk none']='gawk gawk'

      ['c89 none']='c89 gcc-core'

      ['c99 none']='c99 gcc-core'

      ['cat none']='cat coreutils'

      ['cc none']='cc gcc-core'

      ['gawk none']='gawk gawk'

      ['git none']='git git'

      ['gpg1 none']='gpg gnupg'

      ['gpg2 none']='gpg2 gnupg2'

      ['jq none']='jq jq'

      ['make none']='make make'

      ['mv none']='mv coreutils'

      ['sed none']='sed sed'

      ['sort none']='sort coreutils'

      ['ssh none']='ssh openssh'

      ['ssh-keygen none']='ssh-keygen openssh'

      ['sshpass none']='sshpass sshpass'

      ['tar none']='tar tar'

    )

  fi

  sst_install_utility_from_map \
    sst_cygwin_install_utility_map \
    "$@" \
  || sst_barf

}; readonly -f sst_cygwin_install_utility
