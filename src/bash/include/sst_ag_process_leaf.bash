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

# TODO: The first parameter should eventually be removed. Our "target"
#       should just follow along the filesystem structure. For example,
#       if we process foo/bar/baz.ag, we should produce:
#
#             foo_bar_leaves += foo/bar.ag
#             foo_bar_children_nodist += foo/bar
#             foo_leaves += $(foo_bar_leaves)
#             foo_children_nodist += $(foo_bar_children_nodist)
#
#       Note how this walks up the directory chain, so that "make foo"
#       will build everything inside the foo directory.
#

sst_ag_process_leaf_helper() {

  # Bash >=4.2: declare -g    SST_DEBUG
  # Bash >=4.2: declare -g -A sst_ag_process_leaf_seen

  declare    child
  declare    child_slug
  declare    children
  declare    children_nodist
  declare    leaf
  declare    leaves
  declare    target
  declare    target_slug

  sst_expect_argument_count $# 3

  target=$1
  readonly target
  sst_expect_source_path "$target"

  leaf=$2
  readonly leaf
  sst_expect_source_path "$leaf"

  sst_expect_basic_identifier "$3"

  target_slug=$(sst_underscore_slug $target)
  readonly target_slug

  children=${target_slug}_children
  readonly children

  children_nodist=${children}_nodist
  readonly children_nodist

  leaves=${target_slug}_leaves
  readonly leaves

  if ((SST_DEBUG)); then
    if [[ ! -f $leaf ]]; then
      sst_barf "Leaf does not exist: \"$leaf\"."
    fi
  fi

  if [[ ! "${sst_ag_process_leaf_seen[$leaf]+x}" ]]; then
    if [[ $leaf == *.phony.@(ag|ac|am) ]]; then
      # TODO: .phony handling should probably eventually go away. We
      #       really want each leaf to correspond to exactly one child.
      #       Not none, and not more than one. We'll still have "phony"
      #       .ag files, but they should always be above directories
      #       processed by this function.
      # TODO: OK >1 child can be a problem because it makes it hard to
      #       do stuff for the child, but why is 0 a problem? Can't the
      #       caller just check for 0? Can we somehow return a list and
      #       then any number of children is fine? Oh, if we do the
      #       include right here, then that .ag file may do multiple
      #       calls to this very function, then we're in trouble? Maybe
      #       the right thing to do here is a noop? Error?
      child=
      sst_${leaf##*.}_include $leaf
    elif [[ $leaf == *.@(ag|ac|am) ]]; then
      child=${leaf%%.@(ag|ac|am)}
      child_slug=$(sst_underscore_slug $child)
      sst_${leaf##*.}_include $leaf
      sst_am_var_add_unique_word $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    elif [[ $leaf == *.@(im.in|in|im) ]]; then
      child=${leaf%%.@(im.in|in|im)}
      child_slug=$(sst_underscore_slug $child)
      sst_ac_config_file $leaf
      sst_am_var_add_unique_word $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    elif [[ $leaf == *.m4 ]]; then
      child=${leaf%%.m4}
      child_slug=$(sst_underscore_slug $child)
      sst_am_distribute $leaf
      sst_ac_append <<<"GATBPS_M4([$child])"
      sst_am_var_add_unique_word $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    else
      child=$leaf
      sst_am_distribute $leaf
      sst_am_var_add_unique_word $children $child
      sst_am_var_add_unique_word $leaves $leaf
    fi
    sst_ag_process_leaf_seen[$leaf]=$child
  fi

  sst_ag_process_leaf_child=${sst_ag_process_leaf_seen[$leaf]}

}; readonly -f sst_ag_process_leaf_helper

sst_ag_process_leaf() {

  declare    sst_ag_process_leaf_child

  sst_ag_process_leaf_helper "$@"

  eval $3=$sst_ag_process_leaf_child

}; readonly -f sst_ag_process_leaf
