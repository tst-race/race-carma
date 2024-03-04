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

sst_am_install() {

  # Bash >=4.2: declare -g -A sst_am_install_dirs

  declare    base
  declare    dir
  declare    dir_slug
  declare    dst
  declare    dst_a
  declare    dst_b
  declare    primary
  declare    src
  declare    suf
  declare    x
  declare    y

  if ((SST_DEBUG)); then
    sst_expect_not_subshell
    if (($# < 3)); then
      sst_expect_argument_count $# 3-
    fi
  fi

  dst=$1
  readonly dst

  dst_a=${dst%%/*}
  readonly dst_a

  if ((SST_DEBUG)); then
    sst_expect_source_path "$dst"
    x='@(bin'
    x+='|data'
    x+='|dataroot'
    x+='|include'
    x+='|libexec'
    x+=')'
    if [[ $dst_a != $x ]]; then
      sst_barf "dst='$dst' must begin with one of $x."
    fi
  fi

  dst_b=${dst#$dst_a}
  readonly dst_b

  primary=$2
  if [[ "$primary" == - ]]; then
    case $dst_a in bin | libexec)
      primary=SCRIPTS
    ;; include)
      primary=HEADERS
    ;; *)
      primary=DATA
    esac
  fi
  readonly primary

  if ((SST_DEBUG)); then
    x='@(DATA'
    x+='|HEADERS'
    x+='|JAVA'
    x+='|LIBRARIES'
    x+='|LISP'
    x+='|LTLIBRARIES'
    x+='|MANS'
    x+='|PROGRAMS'
    x+='|PYTHON'
    x+='|SCRIPTS'
    x+='|TEXINFOS'
    x+=')'
    if [[ "$primary" != $x ]]; then
      y=$(sst_quote "$primary")
      sst_barf "primary=$y must be one of $x."
    fi
  fi

  base=$3
  readonly base

  if ((SST_DEBUG)); then
    if [[ "$base" != - ]]; then
      sst_expect_source_path "$base"
    fi
  fi

  shift 3
  while (($# > 0)); do

    src=$1
    shift

    if ((SST_DEBUG)); then
      sst_expect_source_path "$src"
    fi

    if [[ -d $src ]]; then
      set -- $src/* "$@"
      continue
    fi

    if ((SST_DEBUG)); then
      if [[ $base != - && $src != $base/* ]]; then
        sst_barf "src='$src' must be below base='$base'."
      fi
    fi

    if [[ -f $src ]]; then
      # TODO: What to do about asdf here?
      sst_ag_process_leaf asdf $src src
    fi

    suf=
    if [[ $base != - ]]; then
      suf=${src#$base}
      suf=${suf%/*}
    fi

    dir=$dst$suf
    dir_slug=${dir//[!0-9A-Z_a-z]/_}

    if [[ ! "${sst_am_install_dirs[$dir]+x}" ]]; then
      if [[ "$dst_b" || "$suf" ]]; then
        sst_am_suspend_if
        sst_am_append <<EOF
${dir_slug}dir = \$(${dst_a}dir)$dst_b$suf
GATBPS_UNINSTALL_MKDIRS += \$(${dir_slug}dir)/mkdir
EOF
        sst_am_restore_if
      fi
      sst_am_install_dirs[$dir]=
    fi

    sst_am_var_add ${dir_slug}_$primary $src

  done

}; readonly -f sst_am_install
