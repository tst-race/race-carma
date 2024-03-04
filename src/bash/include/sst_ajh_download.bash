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

sst_ajh_download() {

  local dir
  local ff
  local tt
  local x1
  local x2
  local x3
  local x4

  case $# in
    1)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  dir=$1
  readonly dir

  for x1 in $dir/**/*.ag.json; do

    sst_expect_ag_json x2 "$x1"
    x3=$(basename $x2)
    x4=$(sst_underscore_slug $x3)

    tt=$(jq .type $x1)

    case $tt in

      null)

        autogen_ac_append <<EOF

]m4_define(
  [${x4}_urls_default],
  [[ ]dnl
[$dir/local/$x3 ]dnl
EOF

        jq -r '
          .urls[]
          | select(type == "string" and . != "")
          | gsub("\\$"; "$][$][")
          | "['\'\\\\\'\''" + . + "'\'\\\\\'\'' ]dnl"
        ' $x1 | autogen_ac_append

        autogen_ac_append <<EOF
])[

case $][{${x4}_urls+x} in
  "")
    ${x4}_urls=']${x4}_urls_default['
  ;;
esac
readonly ${x4}_urls

]AC_ARG_VAR(
  [${x4}_urls],
  [
    the URLs from which to download the
    $x3
    file (default:
    ${x4}_urls=']${x4}_urls_default[')
  ])[

]GATBPS_WGET(
  [$x2],
  [
    [\$(${x4}_urls)],
  ],
  [
EOF

        jq -r '
          .hashes | to_entries[] | "    [" + .key + ":" + .value + "],"
        ' $x1 | autogen_ac_append

        autogen_ac_append <<EOF
  ],
  [clean])[

EOF

      ;;

      \"copy\")

        ff=$(jq -r .file $x1)

        autogen_ac_append <<EOF

]GATBPS_CP(
  [$x2],
  [$ff],
  [file],
  [clean])[

EOF

      ;;

      *)

        sst_barf 'unknown type: %s' "$tt"

      ;;

    esac

  done

}; readonly -f sst_ajh_download
