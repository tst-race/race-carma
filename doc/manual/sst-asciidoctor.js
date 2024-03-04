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

globalThis.sst = globalThis.sst || {};

const body_style = window.getComputedStyle(document.body);

const sst_info_button_size =
    body_style.getPropertyValue("--sst_info_button_size")
        .replace("px", "");

// Source: https://icons.getbootstrap.com/icons/info-circle/
const sst_info_button_open_svg = `
  <svg xmlns="http://www.w3.org/2000/svg" width="${sst_info_button_size}" height="${sst_info_button_size}" fill="currentColor" viewBox="0 0 16 16">
    <path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14zm0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16z"/>
    <path d="m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z"/>
  </svg>
`;

// Source: https://icons.getbootstrap.com/icons/x-circle-fill/
const sst_info_button_close_svg = `
  <svg xmlns="http://www.w3.org/2000/svg" width="${sst_info_button_size}" height="${sst_info_button_size}" fill="currentColor" viewBox="0 0 16 16">
    <path d="M16 8A8 8 0 1 1 0 8a8 8 0 0 1 16 0zM5.354 4.646a.5.5 0 1 0-.708.708L7.293 8l-2.647 2.646a.5.5 0 0 0 .708.708L8 8.707l2.646 2.647a.5.5 0 0 0 .708-.708L8.707 8l2.647-2.646a.5.5 0 0 0-.708-.708L8 7.293 5.354 4.646z"/>
  </svg>
`;

//----------------------------------------------------------------------

function sst_is_screen_hidden($x) {
  return $x.hasClass("sst_screen_hide");
}

function sst_screen_show($x) {
  $x.removeClass("sst_screen_hide");
}

function sst_screen_hide($x) {
  if (!sst_is_screen_hidden($x)) {
    $x.addClass("sst_screen_hide");
  }
}

//----------------------------------------------------------------------

{

  const sst_asciidoctor_css =
      document.getElementById("sst_asciidoctor_css");

  const append_css_rule = function(rule) {
    const sheet = sst_asciidoctor_css.sheet;
    sheet.insertRule(rule, sheet.cssRules.length);
  }

  //--------------------------------------------------------------------
  // KaTeX
  //--------------------------------------------------------------------

  const have_katex = window.renderMathInElement !== undefined;

  if (have_katex) {

    //------------------------------------------------------------------
    // Use the KaTeX fonts everywhere
    //------------------------------------------------------------------

    append_css_rule(`
      :root {
        --sst_main_font: "KaTeX_Main", serif;
      }
    `);

    append_css_rule(`
      :root {
        --sst_code_font: "KaTeX_Typewriter", monospace;
      }
    `);

    //------------------------------------------------------------------
  }
}

//----------------------------------------------------------------------
// Table of contents
//----------------------------------------------------------------------

let sst_min_toc_width = 200;
const sst_min_content_width = 500;

if (window.jQuery) {
  const $body = $("body");
  const $toc = $("#toc");
  const $toc_li = $toc.find("li");
  const $nodes = $toc.children("ul").children("li").find("ul");

  $nodes.each(function() {
    sst_screen_hide($(this));
  });

  $toc_li
      .prepend($("<span>")
                   .addClass("sst_toc_arrow")
                   .addClass("sst_toc_arrow_right")
                   .click(function() {
                     const $node = $(this).closest("li").children("ul");
                     if (sst_is_screen_hidden($node)) {
                       sst_screen_show($node);
                       $(this).removeClass("sst_toc_arrow_right");
                       $(this).addClass("sst_toc_arrow_down");
                     } else {
                       sst_screen_hide($node);
                       $(this).removeClass("sst_toc_arrow_down");
                       $(this).addClass("sst_toc_arrow_right");
                     }
                   }))
      .filter(function() {
        return $(this).children("ul").length == 0;
      })
      .children(".sst_toc_arrow")
      .css("visibility", "hidden");

  $toc_li.each(function() {
    $(this)
        .children(":not(ul)")
        .wrapAll($("<div>").addClass("sst_toc_indent"));
  });

  {
    $toc.resizable({
      handles: "e",
      resize: function(event, ui) {
        ui.size.width += $toc.outerWidth() - $toc.width();
        ui.size.width =
            Math.min(ui.size.width,
                     window.innerWidth - sst_min_content_width);
        ui.size.width = Math.max(ui.size.width, sst_min_toc_width);
      }
    });
    const $resizer = $toc.children(".ui-resizable-e");
    let toc_width = undefined;
    const do_width = function() {
      if ($toc.css("position") == "fixed") {
        const old_toc_width = toc_width;
        let set_toc_width = false;
        if ($toc[0].style.width == "100%") {
          $toc.resizable("enable");
          set_toc_width = true;
        } else {
          toc_width = $toc.outerWidth();
          if (old_toc_width === undefined) {
            sst_min_toc_width = Math.min(sst_min_toc_width, toc_width);
          }
        }
        toc_width = Math.min(toc_width,
                             window.innerWidth - sst_min_content_width);
        toc_width = Math.max(toc_width, sst_min_toc_width);
        set_toc_width = set_toc_width || toc_width != old_toc_width;
        if (set_toc_width) {
          $toc.css("width", toc_width);
        }
        $resizer.css("left", toc_width);
        $body.css("margin-left",
                  sst_is_screen_hidden($toc) ? 0 : toc_width);
      } else {
        $toc.resizable("disable");
        $toc.css("width", "100%");
        $body.css("margin-left", "0");
      }
    };
    do_width();
    $(window).resize(do_width);
    const observer = new MutationObserver(do_width);
    observer.observe($(toc)[0], {attributes: true});
  }

  //--------------------------------------------------------------------
  // Navigation help
  //--------------------------------------------------------------------

  $body.append(`
    <div id="sst_info_button">
      <div id="sst_info_button_open">
        ${sst_info_button_open_svg}
      </div>
      <div id="sst_info_button_close">
        ${sst_info_button_close_svg}
      </div>
    </div>
    <div id="sst_info_panel">

      Shortcut keys
      <ul>
      <li>e: Fully expand the table of contents</li>
      <li>c: Fully collapse the table of contents</li>
      <li>h: Show/hide the table of contents</li>
      </ul>

      Copy mode:
      <select id="sst_copy_mode">
        <option value="normal">
          Normal
        </option>
        <option value="gitlab_markdown">
          GitLab Flavored Markdown
        </option>
      </select>

    </div>
  `);

  const $sst_info_button = $("#sst_info_button");
  const $sst_info_button_open = $("#sst_info_button_open");
  const $sst_info_button_close = $("#sst_info_button_close");
  const $sst_info_panel = $("#sst_info_panel");

  $sst_info_button.click(function() {
    $sst_info_panel.toggle();
    $sst_info_button_open.toggle();
    $sst_info_button_close.toggle();
  });

  //--------------------------------------------------------------------

  $(document).keypress(function(e) {
    switch (e.key) {
      case "e": {
        $nodes.each(function() {
          if (sst_is_screen_hidden($(this))) {
            $(this).closest("li").find(".sst_toc_arrow").click();
          }
        });
      } break;

      case "c": {
        $nodes.each(function() {
          if (!sst_is_screen_hidden($(this))) {
            $(this).closest("li").find(".sst_toc_arrow").click();
          }
        });
      } break;

      case "h": {
        if (sst_is_screen_hidden($toc)) {
          sst_screen_show($toc);
        } else {
          sst_screen_hide($toc);
        }
      } break;
    }
  });
}

//----------------------------------------------------------------------
// Copy event handler
//----------------------------------------------------------------------
//
// At the time of writing this paragraph (2021-10-06), multiple
// selection is only supported by Gecko-based browsers, usually via
// holding down the Ctrl key. Attempting to disable it by handling the
// selectionchange event tends to cause more trouble than it's worth, so
// we don't do it. Our copy event handler will always adjust a multiple
// selection by copying everything between the leftmost and rightmost
// endpoints. This won't match what the user sees on their screen, but
// it's the best overall strategy, as attempting to perfectly union the
// multiple selection also tends to cause more trouble than it's worth,
// especially with KaTeX involved.
//
// TODO: More modes and a dropdown in the info panel to select the mode.
//

document.addEventListener("copy", function(event) {

  const sst_copy_mode = document.getElementById("sst_copy_mode").value;

  if (sst_copy_mode === "normal") {
    return;
  }

  function is_element(node, type) {
    console.assert(node instanceof Node);
    console.assert(type === undefined || typeof type === "string"
                   || type instanceof String || type instanceof Array);
    if (node.nodeType !== Node.ELEMENT_NODE) {
      return false;
    }
    if (type === undefined) {
      return true;
    }
    if (type instanceof Array) {
      for (const t of type) {
        if (is_element(node, t)) {
          return true;
        }
      }
      return false;
    }
    if (type.startsWith(".")) {
      return node.classList.contains(type.substring(1));
    }
    return node.tagName == type.toUpperCase();
  }

  function is_text(node) {
    console.assert(node instanceof Node);
    return node.nodeType === Node.TEXT_NODE
           || node.nodeType === Node.COMMENT_NODE
           || node.nodeType === Node.CDATA_SECTION_NODE;
  }

  function is_katex(node) {
    console.assert(node instanceof Node);
    return is_element(node, ".katex");
  }

  function is_under_pre(node) {
    console.assert(node instanceof Node);
    if (is_element(node, "pre")) {
      return true;
    }
    if (node.parentNode !== null) {
      return is_under_pre(node.parentNode);
    }
    return false;
  }

  function is_ignore(node) {
    console.assert(node instanceof Node);
    if (is_text(node)) {
      if (is_under_pre(node)) {
        return false;
      }
      return /^[\t\n\r \u200B]*$/.test(node.textContent);
    }
    if (is_element(node)) {
      if (node.classList.contains("katex-mathml")) {
        return true;
      }
      if (node.tagName == "A" && node.classList.contains("anchor")) {
        return true;
      }
      if (node.tagName == "SPAN") {
        for (let i = 0; i < node.childNodes.length; ++i) {
          if (!is_ignore(node.childNodes[i])) {
            return false;
          }
        }
        return true;
      }
    }
    return false;
  }

  function offset_in_parent(node) {
    console.assert(node instanceof Node);
    const parent = node.parentNode;
    console.assert(parent !== null);
    for (let i = 0; i < parent.childNodes.length; ++i) {
      if (node === parent.childNodes[i]) {
        return i;
      }
    }
  }

  function is_first_sibling(node) {
    console.assert(node instanceof Node);
    if (node.parentNode === null) {
      return false;
    }
    let x = node.previousSibling;
    while (x !== null && is_ignore(x)) {
      x = x.previousSibling;
    }
    return x === null;
  }

  function is_last_sibling(node) {
    console.assert(node instanceof Node);
    if (node.parentNode === null) {
      return false;
    }
    let x = node.nextSibling;
    while (x !== null && is_ignore(x)) {
      x = x.nextSibling;
    }
    return x === null;
  }

  function is_first_offset(node, i) {
    console.assert(node instanceof Node);
    console.assert(i >= 0);
    if (is_text(node)) {
      console.assert(i <= node.textContent.length);
      return i == 0;
    }
    console.assert(i <= node.childNodes.length);
    if (i == node.childNodes.length) {
      return true;
    }
    return is_first_sibling(node.childNodes[i]);
  }

  function is_last_offset(node, i) {
    console.assert(node instanceof Node);
    console.assert(i >= 0);
    if (is_text(node)) {
      console.assert(i <= node.textContent.length);
      return i == node.textContent.length;
    }
    console.assert(i <= node.childNodes.length);
    if (i == 0) {
      return true;
    }
    return is_last_sibling(node.childNodes[i - 1]);
  }

  function expand_left(node) {
    console.assert(node instanceof Node);
    while (is_first_sibling(node)) {
      node = node.parentNode;
    }
    return node;
  }

  function expand_right(node) {
    console.assert(node instanceof Node);
    while (is_last_sibling(node)) {
      node = node.parentNode;
    }
    return node;
  }

  function get_text(node) {
    let x = node.textContent;
    if (!is_under_pre(node)) {
      x = x.replace(/[\n\r]+/g, " ");
    }
    return x;
  }

  function katex_to_latex(node) {
    return $(node).children(".katex-mathml").find("annotation").text();
  }

  let text = "";

  const selection = window.getSelection();

  if (selection.rangeCount > 0) {

    const [start_container, start_offset] = function() {
      const range = selection.getRangeAt(0);
      let a = range.startContainer;
      let b = range.startOffset;
      if (is_first_offset(a, b)) {
        while (is_first_sibling(a)) {
          b = offset_in_parent(a);
          a = a.parentNode;
        }
      }
      return [a, b];
    }();

    const [end_container, end_offset] = function() {
      const range = selection.getRangeAt(selection.rangeCount - 1);
      let a = range.endContainer;
      let b = range.endOffset;
      if (is_last_offset(a, b)) {
        while (is_last_sibling(a)) {
          b = offset_in_parent(a) + 1;
          a = a.parentNode;
        }
      }
      return [a, b];
    }();

    const common_ancestor =
        $(start_container).parents().has($(end_container)).first()[0];

    let started = false;
    function visit(node) {
      if (is_ignore(node)) {
        return {stop: false, text: ""};
      }
      let text = "";
      if (started) {
        if (node === start_container && node === end_container
            && (!is_first_offset(node, start_offset)
                || !is_last_offset(node, end_offset))) {
          if (is_text(node)) {
            text += get_text(node).substring(start_offset, end_offset);
          } else {
            for (let j = start_offset; j < end_offset; ++j) {
              text += visit(node.childNodes[j]).text;
            }
          }
          return {stop: true, text: text};
        }
        if (node === start_container
            && !is_first_offset(node, start_offset)) {
          if (is_text(node)) {
            text += get_text(node).substring(start_offset);
          } else {
            for (let j = start_offset; j < node.childNodes.length;
                 ++j) {
              text += visit(node.childNodes[j]).text;
            }
          }
          return {stop: false, text: text};
        }
        if (node === end_container
            && !is_last_offset(node, end_offset)) {
          if (is_text(node)) {
            text += get_text(node).substring(0, end_offset);
          } else {
            for (let j = 0; j < end_offset; ++j) {
              text += visit(node.childNodes[j]).text;
            }
          }
          return {stop: true, text: text};
        }
        if (is_text(node)) {
          text += get_text(node);
        } else if (sst_copy_mode === "gitlab_markdown"
                   && is_katex(node)) {
          for (let j = 0; j < node.childNodes.length; ++j) {
            const r = visit(node.childNodes[j]);
            text += r.text;
            if (r.stop) {
              return {stop: true, text: text};
            }
          }
          text = "$`" + katex_to_latex(node) + "`$";
        } else {
          if (expand_left(node) !== start_container
              && is_element(node, [
                   ".content",
                   ".paragraph",
                   ".title",
                   "h1",
                   "h2",
                   "h3",
                   "h4",
                   "h5",
                   "h6",
                   "li",
                 ])) {
            text += "\r\n\r\n";
          }
          if (sst_copy_mode === "gitlab_markdown") {
            if (is_element(node, "h1")) {
              text += "# ";
            } else if (is_element(node, "h2")) {
              text += "## ";
            } else if (is_element(node, "h3")) {
              text += "### ";
            } else if (is_element(node, "h4")) {
              text += "#### ";
            } else if (is_element(node, "h5")) {
              text += "##### ";
            } else if (is_element(node, "h6")) {
              text += "###### ";
            }
          }
          for (let j = 0; j < node.childNodes.length; ++j) {
            const r = visit(node.childNodes[j]);
            text += r.text;
            if (r.stop) {
              return {stop: true, text: text};
            }
          }
          if (sst_copy_mode === "gitlab_markdown") {
            if (is_element(node, "pre")) {
              text = "```\r\n" + text + "\r\n```";
            } else if (is_element(node, "code")
                       && !is_under_pre(node)) {
              text = "`" + text + "`";
            }
          }
        }
        return {stop: node === end_container, text: text};
      }
      if (node !== start_container) {
        for (let j = 0; j < node.childNodes.length; ++j) {
          const r = visit(node.childNodes[j]);
          text += r.text;
          if (r.stop || (started && node === end_container)) {
            return {stop: true, text: text};
          }
        }
        return {stop: false, text: text};
      }
      started = true;
      return visit(node);
    }

    text += visit(common_ancestor).text;
  }

  event.clipboardData.setData("text/plain", text);
  event.preventDefault();
});

//----------------------------------------------------------------------
// Code linking
//----------------------------------------------------------------------

const standard_cpp_headers = [
  "algorithm",
  "any",
  "array",
  "atomic",
  "barrier",
  "bit",
  "bitset",
  "cassert",
  "cctype",
  "cerrno",
  "cfenv",
  "cfloat",
  "charconv",
  "chrono",
  "cinttypes",
  "climits",
  "clocale",
  "cmath",
  "codecvt",
  "compare",
  "complex",
  "concepts",
  "condition_variable",
  "coroutine",
  "csetjmp",
  "csignal",
  "cstdarg",
  "cstddef",
  "cstdint",
  "cstdio",
  "cstdlib",
  "cstring",
  "ctime",
  "cuchar",
  "cwchar",
  "cwctype",
  "deque",
  "exception",
  "execution",
  "filesystem",
  "format",
  "forward_list",
  "fstream",
  "functional",
  "future",
  "initializer_list",
  "iomanip",
  "ios",
  "iosfwd",
  "iostream",
  "istream",
  "iterator",
  "latch",
  "limits",
  "list",
  "locale",
  "map",
  "memory",
  "memory_resource",
  "mutex",
  "new",
  "numbers",
  "numeric",
  "optional",
  "ostream",
  "queue",
  "random",
  "ranges",
  "ratio",
  "regex",
  "scoped_allocator",
  "semaphore",
  "set",
  "shared_mutex",
  "source_location",
  "span",
  "sstream",
  "stack",
  "stdexcept",
  "stop_token",
  "streambuf",
  "string",
  "string_view",
  "strstream",
  "syncstream",
  "system_error",
  "thread",
  "tuple",
  "type_traits",
  "typeindex",
  "typeinfo",
  "unordered_map",
  "unordered_set",
  "utility",
  "valarray",
  "variant",
  "vector",
  "version",
];

const standard_cpp_includes_replace_arg_1 = new RegExp([
  "(#include &lt;)",
  "(" + standard_cpp_headers.join("|") + ")",
  "(&gt;)"
].join(""), "gm");

const standard_cpp_includes_replace_arg_2 = [
  "$1",
  "<a href=\"https://en.cppreference.com/w/cpp/header/$2\">",
  "$2",
  "</a>",
  "$3",
].join("");

const sst_includes_replace_arg_1 = new RegExp([
  "(&lt;)",
  "(sst/[^&]+)",
  "(&gt;)"
].join(""), "gm");

const sst_includes_replace_arg_2 = [
  "$1",
  "<a href=\"https://gitlab.stealthsoftwareinc.com",
  "/stealth/sst/-/blob/master/src/c-cpp/include/$2\">",
  "$2",
  "</a>",
  "$3",
].join("");

for (const code of document.getElementsByTagName("code")) {

  code.innerHTML = code.innerHTML.replace(
    standard_cpp_includes_replace_arg_1,
    standard_cpp_includes_replace_arg_2
  );

  code.innerHTML = code.innerHTML.replace(
    sst_includes_replace_arg_1,
    sst_includes_replace_arg_2
  );

}

//----------------------------------------------------------------------
// Link adjustment based on viewing location
//----------------------------------------------------------------------

for (const f of [

  function (x) {
    if (x.match(new RegExp(
      "^https:\\/\\/stealthsoftwareinc\\.github\\.io\\/"
    ))) {
      return function (y) {
        return y.replace(
          new RegExp([
            "^https:\\/\\/gitlab\\.stealthsoftwareinc\\.com\\/",
            "stealth\\/([^\\/]+)\\/-\\/blob\\/master",
          ].join("")),
          [
            "https://github.com/stealthsoftwareinc/",
            "$1",
            "/blob/master",
          ].join("")
        );
      };
    }
  },

  function (x) {
    if (x.match(new RegExp(
      "^https:\\/\\/stealthsoftwareinc\\.gitlab\\.io\\/"
    ))) {
      return function (y) {
        return y.replace(
          new RegExp([
            "^https:\\/\\/gitlab\\.stealthsoftwareinc\\.com\\/",
            "stealth\\/([^\\/]+)\\/-\\/blob\\/master",
          ].join("")),
          [
            "https://gitlab.com/stealthsoftwareinc/",
            "$1",
            "/-/blob/master",
          ].join("")
        );
      };
    }
  },

]) {
  const g = f(window.location.href);
  if (g) {
    for (const link of document.links) {
      link.href = g(link.href);
    }
    break;
  }
}

//----------------------------------------------------------------------
// sst.iframeExampleOutput
//----------------------------------------------------------------------

globalThis.sst.iframeExampleOutput = function(script, html, {
  height,
} = {}) {
  const showText = "Show output";
  const hideText = "Hide output";
  const parent = script.parentNode;
  const button = document.createElement("button");
  button.className = "sst_iframe_example_output";
  button.textContent = showText;
  button.addEventListener("click", function() {
    if (button.textContent == showText) {
      button.textContent = hideText;
      const iframe = document.createElement("iframe");
      iframe.className = "sst_iframe_example_output";
      if (height !== undefined) {
        iframe.style.height = height;
      }
      parent.insertBefore(iframe, script);
      iframe.contentWindow.document.open();
      iframe.contentWindow.document.write(html);
      iframe.contentWindow.document.close();
    } else {
      button.textContent = showText;
      parent.removeChild(script.previousSibling);
    }
  });
  parent.insertBefore(button, script);
};

//----------------------------------------------------------------------
// Preventing a flash of unstyled content (FOUC)
//----------------------------------------------------------------------
//
// Now that we're done setting everything up, i.e., we've reached the
// bottom of sst-asciidoctor.js, display the page. See the comment in
// sst-asciidoctor.css for more information.
//

document.body.style.display = "block";

//----------------------------------------------------------------------
