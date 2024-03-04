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

      const obs = new IntersectionObserver(
        function(entries, observer) {
          for (const entry of entries) {

            if (entry.isIntersecting) on_visible(entry.target, observer);
          }
        },
        {  threshold: 1.0}
      );

      var lastpos;

      var kestrel_data = {
        "messages": [
//          {"id":123, "sender_id":"Alice", "recipient_id":"Bob", "content":"Hi", "timestamp":1680155434},
//          {"id":124, "sender_id":"Alice", "recipient_id":"Bob", "content":"What's up?", "timestamp":1680155436},
//          {"id":125, "sender_id":"Alice", "recipient_id":"Bob", "content":"We are hanging out next week right?", "timestamp":1680155439},
//          {"id":126, "sender_id":"Bob", "recipient_id":"Alice", "content":"im good", "timestamp":1680155441},
//          {"id":127, "sender_id":"Bob", "recipient_id":"Alice", "content":"How r u?", "timestamp":1680155443},
//          {"id":130, "sender_id":"Alice", "recipient_id":"Bob", "content":"Okay", "timestamp":1680155454},
//          {"id":128, "sender_id":"Bob", "recipient_id":"Alice", "content":"i'll be free on sunday", "timestamp":1680155448},
//          {"id":131, "sender_id":"Alice", "recipient_id":"Bob", "content":"Sunday sounds good!", "timestamp":1680155458},
//          {"id":111, "sender_id":"Charlie", "recipient_id":"Bob", "content":"Hey", "timestamp":1680155428},
        ],
        "contacts": [
//          {"user_id":"Alice", "lookup_status":"ready" /*bootstrapping, incoming*/, "last_message_id":128, "last_message_timestamp": 1680155448},
//          {"user_id":"Charlie", "lookup_status":"ready" /*bootstrapping, incoming*/, "last_message_id":111, "last_message_timestamp": 1680155428},
//          {"user_id":"Eve", "lookup_status":"ready" /*bootstrapping, incoming*/, "last_message_id":null, "last_message_timestamp": null},
//          {"user_id":null, "lookup_status":"bootstrapping" , "bootstrap_id":"0xABADBEEF", "last_message_id":null, "last_message_timestamp": null},
//          {"user_id":"Mallory", "lookup_status":"incoming" , "last_message_id":null, "last_message_timestamp": null}
        ],
        "current_partner_id":null,
        "my_id":"Bob",
        "current_message_id":134
      };
      function incomingMessage(sender_id, content, timestamp) {
        var k = kestrel_data;
        var msg = {};
        msg.id = ++k.current_message_id;
        msg.sender_id = sender_id;
        msg.recipient_id = k.my_id;
        msg.content = content;
        msg.timestamp = timestamp;
        k.messages.push(msg);
        if (!k.contacts.some(x => (x.user_id == sender_id))) { //contact doesn't exist
          addContact(sender_id, "ready");
        }
        var z = kestrel_data.contacts.find(x => x.user_id == sender_id);
        if (!z.last_message_timestamp || timestamp > z.last_message_timestamp) z.last_message_timestamp = timestamp;
        if (k.current_partner_id == sender_id) {
          z.last_message_id = msg.id;
          drawChat(sender_id);
        }
        drawContacts();
      }
      function getContact(uid) {
        return kestrel_data.contacts.find(function (element) { return (element.user_id === uid); });
      }
      function getChats(uid) {
        return kestrel_data.messages.filter(function (element) { return ((element.sender_id === uid) || (element.recipient_id === uid)); });
      }
      function calcNumUnread(remote) {
        var kd = kestrel_data;
        var x = getContact(remote);
        var lmid = x.last_message_id;
        var y = getChats(remote);
        y.sort(function compareFn(a, b) { return (a.id-b.id); });
        var z = y.findIndex(function compare(a) { return (a.id == lmid); });
        //console.log("z="+z+"  ;  remote="+remote+"  ;  lmid=" +lmid);
        //console.log(kestrel_data);
        return (y.length - z - 1);
      }
      function drawContacts() {
        var c = kestrel_data.contacts;
        var cl = $('#kestrel-contact-list')[0];
        cl.replaceChildren();
        c.sort(function(a,b){return ((a.last_message_timestamp===null)-(b.last_message_timestamp===null))||-(a.last_message_timestamp>b.last_message_timestamp)||+(a.last_message_timestamp<b.last_message_timestamp);}); //descending sort null push to end hack
        for (const x of c) {
          const stat = x.lookup_status;
          var cbar = document.createElement("div");
          var cbarclasses = "list-group-item list-group-item-action py-1 border-bottom";
          if (stat === "bootstrapping") {
            cbarclasses += " bootstrapping bg-success bg-opacity-25";
            cbar.setAttribute("bootstrapid",x.bootstrap_id);
          } else if (stat === "incoming") {
            cbarclasses += " incoming bg-danger bg-opacity-25";
            cbar.setAttribute("contactname",x.user_id);
          } else if (stat === "ready") {
            cbarclasses += " ready";
            if (kestrel_data.current_partner_id == x.user_id) cbarclasses += " active";
            cbar.setAttribute("contactname",x.user_id);
            cbar.onclick = function(){openChat(event, x.user_id)};
          }
          cbar.classList.add(...cbarclasses.split(" "));

          var nbar = document.createElement("div");
          const nbarclasses = "d-flex w-100 align-items-center justify-content-between";
          nbar.classList.add(...nbarclasses.split(" "));
          if (stat === "bootstrapping") {
            nbar.innerHTML = `<strong> <i>Bootstrappee: ${x.bootstrap_id}</i>  </strong>`;
            nbar.innerHTML += `<span id="bootstrapme" class="fs-5 fw-semibold"><i class="bi bi-person-gear"></i></span>`;
          } else if (stat === "incoming") {
            nbar.innerHTML = `<strong> ${x.user_id} </strong>`;
            nbar.innerHTML += `<span id="addme" class="fs-5 fw-semibold"><i class="bi bi-person-exclamation"></i></span>`;
          } else if (stat === "ready") {
            nbar.innerHTML = `<strong> ${x.user_id} </strong>`;
            const ur = calcNumUnread(x.user_id);
            if (ur > 0) nbar.innerHTML += `<span id="unread" class="badge bg-danger rounded-pill">${(ur>0)?ur:""}</span>`;
          }
          var lc = document.createElement("div");
          lc.setAttribute("id","lastchat");
          lc.classList.add("small");
          if (x.lookup_status === "ready") {
            const date = new Date(x.last_message_timestamp*1000);
            var lctext = (x.last_message_timestamp === null) ? "-" : date.toLocaleString();
            lc.appendChild(document.createTextNode(lctext));
          } else if (x.lookup_status === "bootstrapping") {
            lc.appendChild(document.createTextNode("Currently bootstrapping"));
          } else if (x.lookup_status === "incoming") {
            lc.appendChild(document.createTextNode("Wants to chat with you"));
          }
          cbar.appendChild(nbar);
          cbar.appendChild(lc);
          cl.appendChild(cbar);

        }
      }

      function drawOwnName() {
        $('#kestrel-self-name').text("Signed in as " + kestrel_data.my_id);
      }
      function nameplate(name) {
        var x = "small p-0 mx-3 my-0 rounded-3 text-muted";
        var newp = document.createElement("p");
        newp.appendChild(document.createTextNode(name));
        newp.setAttribute("class",x);
        return newp;
      }

      function chatchunk(isremote) {
        var newdiv1 = document.createElement("div");
        var newdiv2 = document.createElement("div");
        if (isremote) {
          newdiv1.setAttribute("class","d-flex flex-row justify-content-start remote-message-block");
          newdiv2.setAttribute("class","d-inline-flex flex-column align-items-start");
        } else {
          newdiv1.setAttribute("class","d-flex flex-row justify-content-end self-message-block");
          newdiv2.setAttribute("class","d-inline-flex flex-column align-items-end");
        }
        newdiv1.appendChild(newdiv2);
        return newdiv1;
      }

      function timeplate(time) {
        var x = "small mx-0 mb-0 rounded-3";
        var newp = document.createElement("div");
        var date = new Date(time*1000);
        newp.appendChild(document.createTextNode(date.toLocaleTimeString()));
        newp.setAttribute("class",x);
        return newp;
      }
 function openChat(e, name) {
      kestrel_data.current_partner_id = e.currentTarget.getAttribute("contactname");
      //console.log(kestrel_data.current_partner_id);
      $(".list-group-item").removeClass("active");
      e.currentTarget.classList.add("active");
      $("#kestrel-chat-name").text("Chat with " + name);
      drawChat(kestrel_data.current_partner_id);
      $("#kestrel-sidebar").removeClass("flex-grow-1 flex-sm-grow-0");

      $("#kestrel-chat-main").removeClass("d-none d-sm-none");

      $("#kestrel-chat-none").removeClass("d-none d-sm-flex");

      $("#kestrel-sidebar").addClass("d-none d-sm-flex");

      $("#kestrel-chat-main").addClass("d-flex d-sm-flex");

      $("#kestrel-chat-none").addClass("d-none d-sm-none");

    }

    function closeChat() {
      $(".list-group-item").removeClass("active");

      $("#kestrel-sidebar").removeClass("d-none d-sm-flex");

      $("#kestrel-chat-main").removeClass("d-flex d-sm-flex");

      $("#kestrel-chat-none").removeClass("d-none d-sm-none");

      $("#kestrel-sidebar").addClass("flex-grow-1 flex-sm-grow-0");

      $("#kestrel-chat-main").addClass("d-none d-sm-none");

      $("#kestrel-chat-none").addClass("d-none d-sm-flex");
      kestrel_data.current_partner_id = null;

    }
      function chatmessage(isremote, msg, msg_id, ts) {
        var x;
        if (isremote) x = "chatmsg d-flex flex-column align-items-start justify-content-start p-2 mx-3 mb-1 rounded-3 bg-light"; else x = "chatmsg d-flex flex-column align-items-end justify-content-end p-2 mx-3 mb-1 text-white rounded-3 bg-primary";
        var newp = document.createElement("div");
        newp.appendChild(document.createTextNode(msg));
        newp.setAttribute("class",x);
        newp.setAttribute("msgid",msg_id);
        newp.setAttribute("timestamp",ts);
        return newp;
      }

      function newmessagebanner() {
        var d = document.createElement("div");
        var s = document.createElement("span");
        var i = document.createElement("i");
        i.setAttribute("class","bi bi-chevron-double-down");
        s.setAttribute("class","badge rounded-pill text-bg-success");
        d.setAttribute("class","divider d-flex justify-content-center align-items-center mb-4");
        s.appendChild(i);
        s.appendChild(document.createTextNode(" New Messages Below "));
        d.appendChild(s);
        return d;
      }

      function noprevmsg() {
        var d = document.createElement("div");
        var s = document.createElement("span");
        var i = document.createElement("i");
        i.setAttribute("class","bi bi-info-circle");
        s.setAttribute("class","badge rounded-pill text-bg-secondary");
        d.setAttribute("class","divider d-flex justify-content-center align-items-center mb-4");
        s.appendChild(i);
        s.appendChild(document.createTextNode(" No conversations "));
        d.appendChild(s);
        return d;
      }

      function drawChat(remote) {
        var kd = kestrel_data;
        var x = kd.contacts.find(function (element) { return (element.user_id === remote); });
        //console.log(x.last_message_id);
        var y =  kd.messages.filter(function (element) { return ((element.sender_id === remote) || (element.recipient_id === remote)); });
        //console.log(y);
        y.sort(function compareFn(a, b) { return (a.timestamp-b.timestamp); });
        var chat = $('#chat-card');
        chat[0].replaceChildren();
        var newchildren = [];
        var numchats = y.length;
        if (numchats == 0) {
          newchildren.push(noprevmsg());
        } else {
          var isremote, attr, nm = newmessagebanner();
          var alreadyhasnm = false;
          for (var i = 0; i < numchats; i++) {
            if (y[i].sender_id === remote) {
              isremote = true;
              attr = "sender_id";
            } else {
              isremote = false;
              attr = "recipient_id";
            }
            //console.log("new Block");
            var cc = chatchunk(isremote);
            var ci = cc.getElementsByTagName('div')[0];
            var np = nameplate(isremote?remote:"You");
            ci.appendChild(np);
           //var nm = newmessagebanner();

            while (y[i][attr] === remote && i < numchats) {
              //console.log(y[i]);
              if (y[i].id > x.last_message_id && !alreadyhasnm) { newchildren.push(nm); alreadyhasnm = true; }
              ci.appendChild(chatmessage(isremote, y[i].content, y[i].id, y[i].timestamp));
              i++;
              if (i >= numchats) break;
            }
            ci.lastChild.appendChild(timeplate(y[i-1].timestamp));
            newchildren.push(cc);
            i--;
          }
        }
        chat[0].replaceChildren(...newchildren);
        for (const el of document.querySelectorAll(".chatmsg")) { if (el.getAttribute("msgid") > x.last_message_id) obs.observe(el); }
        if (alreadyhasnm) nm.scrollIntoView(); else  $('#chat-pane')[0].scrollTop=$('#chat-pane')[0].scrollHeight;
      }


function sendchat() {
  var content = $('#my-message').val();
  if (content != "") {
    var msg = {};
    var k = kestrel_data;
    msg.id = ++k.current_message_id;
    msg.sender_id = k.my_id;
    msg.recipient_id = k.current_partner_id;
    msg.content = content;
    msg.timestamp = Math.floor(Date.now() / 1000);
    k.messages.push(msg);
    k.contacts.find(x => (x.user_id === k.current_partner_id)).last_message_id = msg.id;
    k.contacts.find(x => (x.user_id === k.current_partner_id)).last_message_timestamp = msg.timestamp;
    native.kestrel_send(kestrel_data.current_partner_id, content);
  }
  $('#my-message').val("");
  correctChatState();
  drawChat(kestrel_data.current_partner_id);
  drawContacts();

}

function correctChatState() {
 if ($('#my-message').val() === "") {
  var x = $('#send-button');
  x.prop("disabled", true);
  x.attr("class", "btn btn-outline-disabled");
  $('#send-icon').attr("class","bi bi-send");
} else {
  var x = $('#send-button');
  x.prop("disabled", false);
  x.attr("class", "btn btn-outline-primary");
  $('#send-icon').attr("class","bi bi-send-fill");
}
}

function checkChatEnd() {
  if (kestrel_data.current_partner_id !== null) {
    var c = $("#chat-pane").first();
    console.log(lastpos);
    console.log(c.scrollTop());
    if (lastpos === null || lastpos !== c.scrollTop()) {
      lastpos = c.scrollTop();
      if(c.scrollTop() + c.innerHeight() >= c[0].scrollHeight) {
        console.log('end reached');
        drawContacts();
        drawChat(kestrel_data.current_partner_id);
      }
    }
  }
  //setTimeout(() => checkChatEnd(), 2000);
}

function updateContact(remote) {

  const ur = calcNumUnread(remote);
  var badge = $("#kestrel-contact-list").find(`[contactname='${remote}']`).find("#unread");
  if (badge !== null) {
    badge.html((ur>0)?ur:"");
  }
}

function on_visible(element, observer) {

  if (kestrel_data.current_partner_id !== null) {
    var z = kestrel_data.contacts.find(x => x.user_id === kestrel_data.current_partner_id);
    var msgid = element.getAttribute("msgid");
    var ts = element.getAttribute("timestamp")
    if (msgid > z.last_message_id) z.last_message_id = element.getAttribute("msgid");
    if (ts > z.last_message_timestamp) z.last_message_timestamp = element.getAttribute("timestamp");
    console.log(z.last_message_id);
    updateContact(kestrel_data.current_partner_id);
    //drawContacts();
    //drawChat();
  }
  observer.unobserve(element);
}


function addContact(name, status) {
  if (name == null || name === "") return;
  if (kestrel_data.contacts.some(x => (x.user_id == name))) return;
  var contact = {};
  contact.user_id = name;
  contact.lookup_status = status;
  contact.last_message_id = null;
  contact.last_message_timestamp = null;

  kestrel_data.contacts.push(contact);
  drawContacts();
}


function testChatLoop() {
  var c = kestrel_data.contacts;
  var ts = Math.floor(Date.now() / 1000);
  var randuser = c[Math.floor(Math.random() * c.length)].user_id;
  var messages = ["lol", "eh", "?", "..."];
  var randmessage = messages[Math.floor(Math.random() * messages.length)];
  if (randuser != null) incomingMessage(randuser, randmessage, ts);
  setTimeout( () => {testChatLoop()}, Math.floor(Math.random() * 3456));
}
$( document ).ready(function() {
    correctChatState();

    //setTimeout(() => { for (var z = 0; z < 10; z++) {$('#my-message').val("test"); sendchat();} }, 2000);
    //console.log(calcNumUnread("Alice"));
    drawContacts();
    $('#my-message')[0].addEventListener("keypress", function(event) {
      if (event.key === "Enter") {
        sendchat();
      }
    });
    $('#send-button')[0].addEventListener("click", function(event) {
        sendchat();
    });
    $(document).on('input','#my-message',function () {
      correctChatState();
    }
    );
    lastpos = null;
   // testChatLoop();

});

//----------------------------------------------------------------------

const $upload_picker = $("#upload_picker");
const $upload_warning = $("#upload_warning");
const $upload_button_2 = $("#upload_button_2");
const $signed_in_as = $("#signed_in_as");
const $kestrel_self_name = $("#kestrel-self-name");

function display_run() {
  // $upload_warning.removeClass("d-none");
}

function actually_run() {
  native.kestrel_start();
}

function on_start(info) {
  kestrel_data.my_psn_slug = info.psn_slug;
  kestrel_data.my_id = unicodify(unslugify(info.psn_slug));
  $kestrel_self_name.text(kestrel_data.my_id);
  $signed_in_as.removeClass("invisible");

  {
    const x = native.kestrel_clients();
    const n = x.size();
    for (let i = 0; i < n; ++i) {
      const psn_slug = x.get(i);
      if (psn_slug == kestrel_data.my_psn_slug) {
        continue;
      }
      const psn_data = unslugify(psn_slug);
      const psn_text = unicodify(psn_data);
      kestrel_data.contacts.push({
        psn_slug: psn_slug,
        psn_data: psn_data,
        user_id: psn_text,
        lookup_status: "ready",
        last_message_id: null,
        last_message_timestamp: null
      });
    }
  }

  kestrel_data.contacts.sort(
      (a, b) =>
          a.user_id.localeCompare(b.user_id, "en", {numeric: true}));

  drawContacts();

}

function on_stop(error) {
  if (error) {
    console.error(error);
    alert(error);
  }
}

function on_recv_clrmsg(info) {
  incomingMessage(unicodify(unslugify(info.psn_slug)),
                  info.msg_text,
                  info.unix_time_us / 1000000.0);
}

$upload_picker.change(function() {
  show_body();
  display_run();
  rm_f_r(engine_dir);
  mkdir_p(engine_dir);
  unzip($upload_picker[0].files[0], engine_dir, function() {
    actually_run();
  });
});

function main() {
  show_select_config();
  native.kestrel_on_start(on_start.name);
  native.kestrel_on_stop(on_stop.name);
  native.kestrel_on_recv_clrmsg(on_recv_clrmsg.name);
  if (FS.analyzePath(engine_dir).exists) {
    display_run();
    actually_run();
  }
}
