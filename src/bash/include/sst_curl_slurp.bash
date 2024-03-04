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

sst_curl_slurp() {

  declare a
  declare b
  declare get_next
  declare headers
  declare url

  a=$'[\t ]*'
  readonly a

  b=$'[^\t >][^\t >]*'
  readonly b

  get_next=
  get_next+="s/^$a[Ll][Ii][Nn][Kk]$a:.*<$a\\($b\\)$a>$a;"
  get_next+="$a[Rr][Ee][Ll]$a=$a\"$a[Nn][Ee][Xx][Tt]$a\".*/\\1/p"
  readonly get_next

  headers=$sst_root_tmpdir/$FUNCNAME.$BASHPID.headers
  readonly headers

  sst_expect_argument_count $# 1-

  url=$1
  shift

  while [[ "$url" ]]; do
    curl -D "$headers" "$@" -- "$url"
    url=$(tr -d '\r' <"$headers" | sed -n "$get_next")
  done

}; readonly -f sst_curl_slurp
