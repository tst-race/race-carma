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

sst_ag_call_defun_once_macros() {

  declare    x
  declare    xs
  declare    y

  xs=$(sst_grep -l AC_DEFUN_ONCE m4/*.m4)
  readonly xs

  y=m4/CALL_DEFUN_ONCE_MACROS.m4
  readonly y

  sst_expect_not_exist "$y"

  >$y

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

    AC_DEFUN_ONCE([CALL_DEFUN_ONCE_MACROS], [[
    ]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
    { :
  ' >>$y

  sst_push_var IFS

  for x in $xs; do
    x=${x##*/}
    x=${x%.*}
    sst_ihs -2 <<<'
      ]GATBPS_REQUIRE(['"$x"'])[
    ' >>$y
  done

  sst_pop_var IFS

  sst_ihs <<<'
    }]])
  ' >>$y

  sst_ac_append <<<'CALL_DEFUN_ONCE_MACROS'

}; readonly -f sst_ag_call_defun_once_macros
