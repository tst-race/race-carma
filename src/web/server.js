//
// Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS
// IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

"use strict";

const $not_running = $("#not_running");
const $running_as = $("#running_as");
const $running_psn = $("#running_psn");
const $running_psn_text = $("#running_psn_text");
const $running_psn_bar = $("#running_psn_bar");
const $running_spinner = $("#running_spinner");
const $upload_button_1 = $("#upload_button_1");
const $upload_picker = $("#upload_picker");
const $upload_warning = $("#upload_warning");
const $upload_button_2 = $("#upload_button_2");

$upload_button_1.click(function() {
  $upload_picker.val("");
  $upload_button_2.prop("disabled", true);
});

$upload_picker.change(function() {
  $upload_button_2.prop("disabled", false);
});

function display_run() {
  $not_running.addClass("d-none");
  $running_as.removeClass("d-none");
  $running_psn.removeClass("invisible");
  $running_psn_text.addClass("d-none");
  $running_psn_bar.removeClass("d-none");
  $running_spinner.removeClass("invisible");
  $upload_button_1.removeClass("btn-primary");
  // $upload_button_1.addClass("btn-secondary");
  $upload_button_1.addClass("d-none");
  // $upload_warning.removeClass("d-none");
}

function on_start(info) {
  $running_psn_text.text(unicodify(unslugify(info.psn_slug)));
  $running_psn_text.removeClass("d-none");
  $running_psn_bar.addClass("d-none");
}

let running = false;

function actually_run() {
  native.kestrel_start();
  running = true;
}

function on_stop(error) {
  running = false;
  if (error) {
    console.error(error);
    alert(error);
  } else {
    actually_run();
  }
}

$upload_button_2.click(function() {
  display_run();
  rm_f_r(engine_dir);
  mkdir_p(engine_dir);
  unzip($upload_picker[0].files[0], engine_dir, function() {
    flush();
    if (running) {
      native.kestrel_stop();
    } else {
      actually_run();
    }
  });
});

function main() {
  show_body();
  native.kestrel_on_start(on_start.name);
  native.kestrel_on_stop(on_stop.name);
  if (FS.analyzePath(engine_dir).exists) {
    display_run();
    actually_run();
  }
}
