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

sst_ag_install_bash_library() {

  # Bash >=4.2: declare -g    sst_ag_install_bash_library_i

  declare    dstdir
  declare    target
  declare    srcdir
  declare    x
  declare    y

  srcdir=$1
  readonly srcdir

  dstdir=bash
  readonly dstdir

  : ${sst_ag_install_bash_library_i=0}
  ((++sst_ag_install_bash_library_i))

  target=sst_ag_install_bash_library_$sst_ag_install_bash_library_i
  readonly target

  x=$srcdir/sst.bash
  sst_ag_process_leaf $target $x y
  sst_am_append <<EOF
${target}dir = \$(pkgdatadir)/$dstdir
${target}_DATA = $srcdir/sst.bash
EOF

  for x in $srcdir/{include,scripts}/**/; do
    x=${x#$srcdir/}
    y=$x
    x=${x%/}
    x=${x//[!0-9A-Z_a-z]/_}
    sst_am_append <<EOF
${target}_${x}dir = \$(pkgdatadir)/$dstdir/$y
${target}_${x}_DATA =
EOF
  done

  for x in $srcdir/{include,scripts}/**/*.bash*; do
    sst_ag_process_leaf $target $x y
    x=${x#$srcdir/}
    x=${x%/*}
    x=${x//[!0-9A-Z_a-z]/_}
    sst_am_append <<EOF
${target}_${x}_DATA += $y
EOF
  done

}; readonly -f sst_ag_install_bash_library
