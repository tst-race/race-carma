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

autogen_texinfo() {

  local dd
  local x1
  local x2
  local x3
  local y1
  local y2
  local y2s

  case $# in
    0 | 1)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  x1=$1
  readonly x1
  expect_safe_path "$x1"
  case $x1 in
    *.texi)
    ;;
    *)
      sst_barf '$1 must end in .texi: %s' $x1
    ;;
  esac
  shift

  x2=$(echo $x1 | sed 's/\.texi$//')
  readonly x2

  x3=$(echo $x2 | sed 's/[^0-9A-Z_a-z]/_/g')
  readonly x3

  cat >$x2.am <<EOF
${x3}_TEXINFOS =
EOF

  for dd; do

    expect_safe_path "$dd"

    y2s=:

    for y1 in \
      $dd/*.css \
      $dd/*.texi \
      $dd/*.texi.in \
    ; do

      expect_safe_path "$y1"

      if [[ $y1 == $x1 ]]; then
        continue
      fi

      case $y1 in
        *.texi.in)
          y2=$(echo $y1 | sed 's/\.in$//')
        ;;
        *)
          y2=$y1
        ;;
      esac

      case $y2s in
        *:$y2:*)
          continue
        ;;
      esac

      printf '%s_TEXINFOS += %s\n' $x3 $y2 >>$x2.am

      y2s=$y2s$y2:

    done

  done

  sst_am_include $x2.am

}; readonly -f autogen_texinfo
