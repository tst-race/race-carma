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

sst_ubuntu_install_utility() {

  # Bash >=4.2: declare -g -A sst_ubuntu_install_utility_map
  # Bash >=4.2: declare -g sst_ubuntu_install_utility_once

  if ((!${sst_ubuntu_install_utility_once-0})); then
    sst_ubuntu_install_utility_once=1

    #-------------------------------------------------------------------

    sst_ubuntu_install_docker_compose() {
      sst_unix_like_install_docker_compose \
        curl \
        jq \
        sudo \
      || sst_barf
    }; readonly -f sst_ubuntu_install_docker_compose

    #-------------------------------------------------------------------

    sst_ubuntu_install_utility_map=(

      ['awk 16.04']='gawk gawk'
      ['awk 18.04']='gawk gawk'
      ['awk 20.04']='gawk gawk'
      ['awk 22.04']='gawk gawk'
      ['awk unknown']='gawk gawk'

      ['c89 16.04']='c89 gcc'
      ['c89 18.04']='c89 gcc'
      ['c89 20.04']='c89 gcc'
      ['c89 22.04']='c89 gcc'
      ['c89 unknown']='c89 gcc'

      ['c99 16.04']='c99 gcc'
      ['c99 18.04']='c99 gcc'
      ['c99 20.04']='c99 gcc'
      ['c99 22.04']='c99 gcc'
      ['c99 unknown']='c99 gcc'

      ['cat 16.04']='cat coreutils'
      ['cat 18.04']='cat coreutils'
      ['cat 20.04']='cat coreutils'
      ['cat 22.04']='cat coreutils'
      ['cat unknown']='cat coreutils'

      ['cc 16.04']='cc gcc'
      ['cc 18.04']='cc gcc'
      ['cc 20.04']='cc gcc'
      ['cc 22.04']='cc gcc'
      ['cc unknown']='cc gcc'

      ['curl 16.04']='curl curl'
      ['curl 18.04']='curl curl'
      ['curl 20.04']='curl curl'
      ['curl 22.04']='curl curl'
      ['curl unknown']='curl curl'

      ['docker-compose 16.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose 18.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose 20.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose 22.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose unknown']='sst_ubuntu_install_docker_compose'

      ['gawk 16.04']='gawk gawk'
      ['gawk 18.04']='gawk gawk'
      ['gawk 20.04']='gawk gawk'
      ['gawk 22.04']='gawk gawk'
      ['gawk unknown']='gawk gawk'

      ['git 16.04']='git git'
      ['git 18.04']='git git'
      ['git 20.04']='git git'
      ['git 22.04']='git git'
      ['git unknown']='git git'

      ['gpg1 16.04']='gpg gnupg'
      ['gpg1 18.04']='gpg1 gnupg1'
      ['gpg1 20.04']='gpg1 gnupg1'
      ['gpg1 22.04']='gpg1 gnupg1'
      ['gpg1 unknown']='gpg1 gnupg1'

      ['gpg2 16.04']='gpg2 gnupg2'
      ['gpg2 18.04']='gpg2 gnupg2'
      ['gpg2 20.04']='gpg2 gnupg2'
      ['gpg2 22.04']='gpg2 gnupg2'
      ['gpg2 unknown']='gpg2 gnupg2'

      ['jq 16.04']='jq jq'
      ['jq 18.04']='jq jq'
      ['jq 20.04']='jq jq'
      ['jq 22.04']='jq jq'
      ['jq unknown']='jq jq'

      ['make 16.04']='make make'
      ['make 18.04']='make make'
      ['make 20.04']='make make'
      ['make 22.04']='make make'
      ['make unknown']='make make'

      ['mv 16.04']='mv coreutils'
      ['mv 18.04']='mv coreutils'
      ['mv 20.04']='mv coreutils'
      ['mv 22.04']='mv coreutils'
      ['mv unknown']='mv coreutils'

      ['sed 16.04']='sed sed'
      ['sed 18.04']='sed sed'
      ['sed 20.04']='sed sed'
      ['sed 22.04']='sed sed'
      ['sed unknown']='sed sed'

      ['sort 16.04']='sort coreutils'
      ['sort 18.04']='sort coreutils'
      ['sort 20.04']='sort coreutils'
      ['sort 22.04']='sort coreutils'
      ['sort unknown']='sort coreutils'

      ['ssh 16.04']='ssh openssh-client'
      ['ssh 18.04']='ssh openssh-client'
      ['ssh 20.04']='ssh openssh-client'
      ['ssh 22.04']='ssh openssh-client'
      ['ssh unknown']='ssh openssh-client'

      ['ssh-keygen 16.04']='ssh-keygen openssh-client'
      ['ssh-keygen 18.04']='ssh-keygen openssh-client'
      ['ssh-keygen 20.04']='ssh-keygen openssh-client'
      ['ssh-keygen 22.04']='ssh-keygen openssh-client'
      ['ssh-keygen unknown']='ssh-keygen openssh-client'

      ['sshpass 16.04']='sshpass sshpass'
      ['sshpass 18.04']='sshpass sshpass'
      ['sshpass 20.04']='sshpass sshpass'
      ['sshpass 22.04']='sshpass sshpass'
      ['sshpass unknown']='sshpass sshpass'

      ['sudo 16.04']='sudo sudo'
      ['sudo 18.04']='sudo sudo'
      ['sudo 20.04']='sudo sudo'
      ['sudo 22.04']='sudo sudo'
      ['sudo unknown']='sudo sudo'

      ['tar 16.04']='tar tar'
      ['tar 18.04']='tar tar'
      ['tar 20.04']='tar tar'
      ['tar 22.04']='tar tar'
      ['tar unknown']='tar tar'

      ['wget 16.04']='wget wget'
      ['wget 18.04']='wget wget'
      ['wget 20.04']='wget wget'
      ['wget 22.04']='wget wget'
      ['wget unknown']='wget wget'

    )

    #-------------------------------------------------------------------

  fi

  sst_install_utility_from_map \
    sst_ubuntu_install_utility_map \
    "$@" \
  || sst_barf

}; readonly -f sst_ubuntu_install_utility
