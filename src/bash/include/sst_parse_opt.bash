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

sst_parse_opt_helper() {

  local array
  local i
  local -r integer_regex='^(0|[1-9][0-9]*)$'
  local is_long
  local matcher
  local n
  local option
  local -a options
  local prefix
  local style
  local unpack_array

  sst_expect_argument_count $# 2-

  if [[ $# != 0 && "$1" == : ]]; then
    unpack_array=1
    shift
  else
    unpack_array=
  fi
  readonly unpack_array

  if [[ $# == 0 ]]; then
    sst_barf
  elif [[ "$1" == -* || "$1" == *'()' ]]; then
    prefix=
  else
    prefix=$1
    shift
    sst_expect_basic_identifier "$prefix"
  fi
  readonly prefix

  if [[ $# == 0 ]]; then
    sst_barf
  elif [[ "$1" == -* ]]; then
    options=()
    matcher=
    while [[ $# != 0 && "$1" == -* ]]; do
      if [[ ${#1} != 1 && "${1:1:1}" == - ]]; then
        is_long=1
      else
        is_long=
      fi
      if ((is_long)); then
        if [[ "$1" == *=* ]]; then
          sst_barf
        fi
      else
        if [[ ${#1} != 2 ]]; then
          sst_barf
        fi
      fi
      options+=("$1")
      shift
    done
  elif [[ "$1" == *'()' ]]; then
    options=()
    matcher=${1%'()'}
    sst_expect_basic_identifier "$matcher"
  else
    sst_barf
  fi
  readonly options
  readonly matcher

  if [[ $# == 0 || "$1" == :* ]]; then
    style=required
  else
    style=$1
    shift
    case $style in
      required | permitted | forbidden)
        :
      ;;
      *)
        sst_barf
      ;;
    esac
  fi
  readonly style

  if [[ $# == 0 ]]; then
    sst_barf
  elif [[ "$1" == : ]]; then
    array=
    shift
  elif [[ "$1" == :* ]]; then
    array=${1:1}
    shift
    if [[ "$array" == *:* ]]; then
      i=${array##*:}
      array=${array%:*}
    else
      i=0
    fi
    sst_expect_basic_identifier "$array"
    if [[ ! "$i" =~ $integer_regex ]]; then
      sst_barf
    fi
    if ((unpack_array)); then
      printf '%s\n' '
        local '$array'_i='$i'
        local '$array'_n=${#'$array'[@]}
        # These two if statements are verifying that i < n, with
        # arbitrary precision.
        if ((${#'$array'_i} > ${#'$array'_n})); then
          sst_barf
        fi
        if [[ ${#'$array'_i} == ${#'$array'_n} \
              && ! $'$array'_i < $'$array'_n ]]; then
          sst_barf
        fi
        {
          printf \%s a=
          sst_quote "${'$array'['$i']}"
          if (('$i' < '$array'_n - 1)); then
            printf \%s b=
            sst_quote "${'$array'['$i' + 1]}"
          fi
        } >"$1"
      '
      return
    fi
  else
    sst_barf
  fi
  readonly array
  readonly i

  if [[ $# == 0 ]]; then
    sst_barf
  fi

  if [[ "$matcher" ]]; then
    n=$("$matcher" "$1")
  else
    n=0
    for option in "${options[@]}"; do
      if [[ "$1" == "$option"* ]]; then
        n=${#option}
        break
      fi
    done
  fi
  readonly n

  if ((n == 0)); then
    eval ${prefix}got=
    unset ${prefix}opt
    unset ${prefix}arg
    eval ${prefix}pop=
    return
  fi

  if [[ ${#1} != 1 && "${1:1:1}" == - ]]; then
    is_long=1
  else
    is_long=
  fi

  if ((${#1} == n)); then
    if [[ $style == required ]]; then
      if (($# < 2)); then
        sst_barf "option requires an argument: $1"
      fi
      eval ${prefix}got=1
      eval ${prefix}opt='$1'
      eval ${prefix}arg='$2'
      if [[ "$array" ]]; then
        eval ${prefix}pop=\''
          '$array'=(
            "${'$array'[@]:0:'$i'}"
            "${'$array'[@]:$(('$i' + 1))}"
          )
        '\'
      else
        eval ${prefix}pop=\''
          shift
        '\'
      fi
    else
      eval ${prefix}got=1
      eval ${prefix}opt='$1'
      unset ${prefix}arg
      if [[ "$array" ]]; then
        eval ${prefix}pop=\''
          '$array'['$i']=
        '\'
      else
        eval ${prefix}pop=\''
          shift
          set "" "$@"
        '\'
      fi
    fi
    return
  fi

  if ((is_long)); then
    if [[ "${1:$n:1}" != = ]]; then
      eval ${prefix}got=
      unset ${prefix}opt
      unset ${prefix}arg
      eval ${prefix}pop=
      return
    fi
    if [[ $style == forbidden ]]; then
      sst_barf "option forbids an argument: ${1%%=*}"
    fi
    eval ${prefix}got=1
    eval ${prefix}opt='${1%%=*}'
    eval ${prefix}arg='${1#*=}'
    if [[ "$array" ]]; then
      eval ${prefix}pop=\''
        '$array'['$i']=$'$prefix'arg
      '\'
    else
      eval ${prefix}pop=\''
        shift
        set x "$'$prefix'arg" "$@"
        shift
      '\'
    fi
    return
  fi

  if [[ $style == forbidden ]]; then
    eval ${prefix}got=1
    eval ${prefix}opt='${1:0:2}'
    unset ${prefix}arg
    if [[ "$array" ]]; then
      eval ${prefix}pop=\''
        '$array'['$i']=-${'$array'['$i']:2}
        '$array'=(
          "${'$array'[@]:0:'$i'}"
          ""
          "${'$array'[@]:'$i'}"
        )
      '\'
    else
      eval ${prefix}pop=\''
        sst_parse_opt_tmp=-${1:2}
        shift
        set "" "$sst_parse_opt_tmp" "$@"
      '\'
    fi
  else
    eval ${prefix}got=1
    eval ${prefix}opt='${1:0:2}'
    eval ${prefix}arg='${1:2}'
    if [[ "$array" ]]; then
      eval ${prefix}pop=\''
        '$array'['$i']=$'$prefix'arg
      '\'
    else
      eval ${prefix}pop=\''
        shift
        set x "$'$prefix'arg" "$@"
        shift
      '\'
    fi
  fi

}; readonly -f sst_parse_opt_helper

sst_parse_opt() {

  sst_parse_opt_helper : "$@" >"$sst_root_tmpdir/$FUNCNAME.$BASHPID.x"
  if [[ -s "$sst_root_tmpdir/$FUNCNAME.$BASHPID.x" ]]; then
    . \
      "$sst_root_tmpdir/$FUNCNAME.$BASHPID.x" \
      "$sst_root_tmpdir/$FUNCNAME.$BASHPID.y" \
    ;
    local a b
    a=$sst_root_tmpdir/$FUNCNAME.$BASHPID.y
    a=$(cat "$a")
    eval "$a"
    sst_parse_opt_helper "$@" ${a+"$a"} ${b+"$b"}
  fi

}; readonly -f sst_parse_opt
