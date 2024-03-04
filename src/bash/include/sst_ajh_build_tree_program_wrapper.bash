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

sst_ajh_build_tree_program_wrapper() {

  local ag_json
  local wrappee
  local wrapper

  for ag_json; do

    sst_expect_ag_json wrapper "$ag_json"

    sst_jq_get_string $ag_json wrappee
    if [[ ! "$wrappee" ]]; then
      wrappee=$wrapper.wrappee/${wrapper##*/}
    else
      sst_expect_source_path "$wrappee"
    fi

    sst_mkdir_p_only $wrapper.im

    sst_ihs <<<"
      #! /bin/sh -

      USE_BUILD_TREE=1
      readonly USE_BUILD_TREE
      export USE_BUILD_TREE

      wrappee='{@}abs_builddir{@}/$wrappee'
      readonly wrappee

      case \$# in 0)
        exec \"\$wrappee\"
      ;; *)
        exec \"\$wrappee\" \"\$@\"
      esac
    " >$wrapper.im

    chmod +x $wrapper.im

    sst_ac_config_file $wrapper.im

    sst_am_distribute $wrapper.im

    # TODO: Use some sst_am_var_add_* function that verifies that
    # $wrapper either exists as a file or doesn't exist, and does
    # deduplication?
    sst_am_var_add noinst_SCRIPTS $wrapper

  done

}; readonly -f sst_ajh_build_tree_program_wrapper
