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

sst_ajh_java_program_wrapper() {

  local ag_json
  local class
  local jar
  local jar_base
  local schema
  local src
  local wrapper
  local x

  for ag_json; do

    sst_expect_ag_json wrapper "$ag_json"

    sst_jq_get_string $ag_json class

    sst_jq_get_string $ag_json jar

    sst_jq_get_string $ag_json src

    if [[ "$src" == "" ]]; then
      src=$sst_root_tmpdir/$FUNCNAME.src
      if [[ ! -f "$src" ]]; then
        sst_ihs <<<'
          #! /bin/sh -

          #
          # We use -ea in default_javaflags because it'\''s a good default approach:
          # slower code with more diagnostics is often preferable to faster code
          # with fewer diagnostics. When we really do want to go as fast as
          # possible, we can use -da in JAVAFLAGS.
          #
          # We export CLASSPATH because this allows it to be overridden by using
          # -cp in JAVAFLAGS. We could accomplish the same thing by using -cp in
          # default_javaflags, but exporting CLASSPATH is better because it
          # doesn'\''t have any quoting concerns.
          #

          case $JAVA in "")
            JAVA=java
          esac
          readonly JAVA

          x=
          x=$x'\'' -ea'\''
          default_javaflags=$x
          readonly default_javaflags

          readonly JAVAFLAGS

          x=
          x="$x $JAVA"
          x="$x -XshowSettings:property"
          x="$x -version"
          x="$x 2>&1"
          x=`eval "$x"` || exit $?
          case $x in *"path.separator = ;"*)
            path_separator=";"
          ;; *)
            path_separator=:
          esac
          readonly path_separator

          case $CLASSPATH in ?*)
            CLASSPATH=$CLASSPATH$path_separator
          esac
          case ${USE_BUILD_TREE-} in 1)
            x='\''{@}abs_builddir{@}/%jar%.build_tree_classpath'\''
            CLASSPATH=$CLASSPATH`cat "$x"` || exit $?
          ;; *)
            x='\''{@}javadir{@}/%jar_base%.classpath'\''
            CLASSPATH=$CLASSPATH`cat "$x"` || exit $?
          esac
          case $path_separator in ";")
            CLASSPATH=`cygpath -l -p -w -- "$CLASSPATH"` || exit $?
          esac
          readonly CLASSPATH
          export CLASSPATH

          x=
          x="$x $JAVA"
          x="$x $default_javaflags"
          x="$x $JAVAFLAGS"
          x="$x "'\''%class%'\''
          case $# in 0)
            :
          ;; *)
            x="$x \"\$@\""
          esac
          eval exec "$x"
        ' >"$src"
      fi
    else
      sst_expect_source_path "$src"
    fi

    jar_base=${jar##*/}

    sed "
      s|%class%|$class|g
      s|%jar%|$jar|g
      s|%jar_base%|$jar_base|g
    " <"$src" >$wrapper.im.in

    chmod +x $wrapper.im.in

    sst_ac_config_file $wrapper.im.in

    # TODO: Use some sst_am_var_add_* function that verifies that
    # $wrapper either exists as a file or doesn't exist, and does
    # deduplication?
    sst_am_var_add bin_SCRIPTS $wrapper

  done

}; readonly -f sst_ajh_java_program_wrapper
