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

sst_ag_define_ordering_macros() {

  declare    x
  declare    x_file
  declare    y
  declare    y_file
  declare -A y_to_xs
  declare    ys

  if (($# != 0)); then
    sst_expect_argument_count $# 0
  fi

  y_to_xs=()

  for x_file in m4/*.m4; do
    x=$x_file
    x=${x##*/}
    x=${x%.*}
    ys=$(
      sed -n '
        s/.*GATBPS_BEFORE(\[\$0\], \[\([^]]*\)\]).*/\1/p
      ' $x_file
    )
    for y in $ys; do
      y_to_xs[$y]+=" $x"
    done
  done

  for y in ${!y_to_xs[@]}; do
    y_file=m4/$y.m4
    if [[ ! -e $y_file ]]; then

      sst_info "Generating: $y_file"

      >$y_file

      sst_ihs <<<'
        dnl
        dnl Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
        dnl
        dnl Licensed under the Apache License, Version 2.0 (the "License");
        dnl you may not use this file except in compliance with the License.
        dnl You may obtain a copy of the License at
        dnl
        dnl     http://www.apache.org/licenses/LICENSE-2.0
        dnl
        dnl Unless required by applicable law or agreed to in writing,
        dnl software distributed under the License is distributed on an "AS
        dnl IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
        dnl express or implied. See the License for the specific language
        dnl governing permissions and limitations under the License.
        dnl
        dnl SPDX-License-Identifier: Apache-2.0
        dnl

        AC_DEFUN_ONCE(['$y'], [
        GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
        { :
      ' >>$y_file

      for x in ${y_to_xs[$y]}; do
        sst_ihs -2 <<<'
          GATBPS_REQUIRE(['$x'])
        ' >>$y_file
      done

      sst_ihs <<<'
        }])
      ' >>$y_file

    fi
  done

}; readonly -f sst_ag_define_ordering_macros
