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

sst_install_utility() {

  # Bash >=4.2: declare -g    sst_distro
  # Bash >=4.2: declare -g -A sst_utility_overrides
  # Bash >=4.2: declare -g -A sst_utility_prefixes
  # Bash >=4.2: declare -g -A sst_utility_programs
  # Bash >=4.2: declare -g -A sst_utility_seen
  # Bash >=4.2: declare -g -A sst_utility_suffixes

  declare    install
  declare    override
  declare -a utilities
  declare    utility

  utilities=()
  for utility; do
    if [[ "${sst_utility_seen[$utility]-}" ]]; then
      continue
    fi
    sst_utility_seen[$utility]=1
    utilities+=("$utility")
  done
  readonly utilities

  sst_get_distro || sst_err_trap "$sst_last_command"

  #
  # Find the utility installation function for this distro.
  #

  install=sst_${sst_distro}_install_utility
  if [[ "$(type -t $install)" != function ]]; then
    install=
  fi
  readonly install

  #
  # If we have a utility installation function for this distro, use it
  # to install all of the requested utilities in one go. Calling it once
  # for all of the requested utilities instead of once per utility helps
  # reduce the number of possible user prompts.
  #

  if [[ "$install" ]]; then
    $install "${utilities[@]}" || sst_err_trap "$sst_last_command"
  fi

  for utility in "${utilities[@]}"; do

    #
    # If we don't have a utility installation function for this distro,
    # use the virtual utility name and hope for the best.
    #

    if [[ ! "$install" ]]; then
      sst_utility_prefixes[$utility]=
      sst_utility_programs[$utility]=$utility
      sst_utility_suffixes[$utility]=
    fi

    #-------------------------------------------------------------------
    # Environment variable override
    #-------------------------------------------------------------------

    override=$(sst_environment_slug <<<"$utility")
    eval override=\${$override-}
    if [[ "$override" ]]; then
      sst_utility_overrides[$utility]=$override
    fi

    #-------------------------------------------------------------------
    # The utility wrapper function
    #-------------------------------------------------------------------
    #
    # This function should behave correctly under errexit suspension, as
    # utilities are often used with idioms such as "utility || s=$?" and
    # "if utility; then".
    #
    # This function should use a prefix for any local variable names.
    # This prevents collisions with any variables in the eval arguments.
    #
    # In some versions of Bash, "command foo" fails to trigger the ERR
    # trap when foo fails. We work around this by using sst_set_exit to
    # make sure the trap is triggered, and by using sst_bash_command to
    # override the failed command printed by the trap.
    #

    eval $utility'() {

      # Bash >=4.2: declare -g sst_bash_command
      # Bash >=4.2: declare -g -A sst_utility_overrides
      # Bash >=4.2: declare -g -A sst_utility_prefixes
      # Bash >=4.2: declare -g -A sst_utility_programs
      # Bash >=4.2: declare -g -A sst_utility_suffixes

      declare sst_utility_status

      if [[ "${sst_utility_overrides['$utility']-}" ]]; then

        eval " \
          ${sst_utility_overrides['$utility']} \
        "'\'' "$@"'\''

      else

        eval " \
          ${sst_utility_prefixes['$utility']} \
          command \
          ${sst_utility_programs['$utility']} \
          ${sst_utility_suffixes['$utility']} \
        "'\'' "$@"'\'' || {

          sst_bash_command=$sst_last_command sst_utility_status=$?

          sst_set_exit $sst_utility_status

          #
          # If we get to this point, i.e., if we get past the above call
          # to sst_set_exit, it means that errexit is suspended. In this
          # case, we need to unset sst_bash_command so we do not clobber
          # the failed command printed by a future invocation of the ERR
          # trap, and we need to call sst_set_exit again to maintain the
          # exit status.
          #

          unset sst_bash_command
          sst_set_exit $sst_utility_status

        }

      fi

    }; readonly -f '$utility

    #-------------------------------------------------------------------

  done

}; readonly -f sst_install_utility
