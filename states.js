/* states.js — controller for the states gallery.
   Reads window.STATES / window.STATE_SECTIONS (state-anims.js), builds the
   featured stage + sectioned grid, handles tempo / shape / tint / replay. */
(function () {
  "use strict";

  var STATES = window.STATES || [];
  var SECTIONS = window.STATE_SECTIONS || [];

  var root = document.documentElement;
  var featStage = document.getElementById("featStage");
  var fname = document.getElementById("fname");
  var fchips = document.getElementById("fchips");
  var fdesc = document.getElementById("fdesc");
  var grid = document.getElementById("grid");
  var spd = document.getElementById("spd");
  var spdVal = document.getElementById("spdVal");

  var soundBtn = document.getElementById("sound");
  var SE = window.SoundEngine;

  var state = {
    id: STATES.length ? STATES[2].id : null, // start on A3_idle: the "home" state
    spd: parseFloat(spd.value),
    shape: "square",
    sound: true,
  };

  function stageClass(id) {
    return "stage " + state.shape + " st-" + id;
  }

  /* fire the state's sound (one-shot voicing; loop states keep a bed going).
     Only called from real user gestures — select / replay / sound toggle —
     so the AudioContext can start (browsers block audio without a gesture). */
  function playCurrent() {
    if (!SE || !state.sound || !state.id) return;
    SE.enter(state.id, state.spd);
  }

  function applySpd() {
    root.style.setProperty("--spd", state.spd);
    spdVal.textContent = "× " + state.spd.toFixed(2);
  }

  // every stage carries the eyes plus two overlays (clock, notepad) that the
  // state classes switch on for A4_clock / B2_listening.
  var STAGE_HTML =
    '<div class="eyes">' +
      '<div class="eye left"><div class="dot"></div></div>' +
      '<div class="eye right"><div class="dot"></div></div>' +
    '</div>' +
    '<div class="clock"><b class="hh">--</b><b class="cl">:</b><b class="mm">--</b></div>' +
    '<div class="notes">' +
      '<div class="pad"></div>' +
      '<div class="ln l1"></div><div class="ln l2"></div>' +
      '<div class="write"></div><div class="pen"></div>' +
    '</div>';

  /* feed the live wall-clock time into every .clock on the page (1 Hz) */
  function tickClock() {
    var d = new Date();
    var hh = ("0" + d.getHours()).slice(-2);
    var mm = ("0" + d.getMinutes()).slice(-2);
    Array.prototype.forEach.call(document.querySelectorAll(".clock"), function (c) {
      var h = c.querySelector(".hh"), m = c.querySelector(".mm");
      if (h) h.textContent = hh;
      if (m) m.textContent = mm;
    });
  }

  function renderGrid() {
    grid.innerHTML = "";
    SECTIONS.forEach(function (s) {
      var head = document.createElement("div");
      head.className = "sechead";
      head.textContent = s.title;
      grid.appendChild(head);

      STATES.filter(function (a) { return a.sec === s.key; }).forEach(function (a) {
        var card = document.createElement("div");
        card.className = "card" + (a.id === state.id ? " active" : "");
        card.dataset.id = a.id;

        var thumb = document.createElement("div");
        thumb.className = "thumb";
        var stage = document.createElement("div");
        stage.className = stageClass(a.id);
        stage.innerHTML = STAGE_HTML;
        thumb.appendChild(stage);

        card.appendChild(thumb);
        card.insertAdjacentHTML(
          "beforeend",
          '<div class="cname">' + a.name + "</div>" +
          '<div class="cmeta">' + a.id + " · " + a.type + "</div>"
        );
        card.addEventListener("click", function () { select(a.id); });
        grid.appendChild(card);
      });
    });
  }

  function applyFeatured() {
    if (!state.id) return;
    featStage.className = stageClass(state.id);
    var a = find(state.id);
    if (a) {
      fname.textContent = a.id + " · " + a.name;
      fchips.innerHTML =
        '<span class="chip">' + a.type + "</span>" +
        '<span class="chip">' + a.dur + "</span>" +
        '<span class="chip light">light ' + a.light + "</span>";
      fdesc.textContent = a.desc;
    }
    Array.prototype.forEach.call(grid.querySelectorAll(".card"), function (c) {
      c.classList.toggle("active", c.dataset.id === state.id);
    });
  }

  function find(id) {
    for (var i = 0; i < STATES.length; i++) if (STATES[i].id === id) return STATES[i];
    return null;
  }

  function select(id) { state.id = id; applyFeatured(); playCurrent(); }

  function refreshAllStages() {
    applyFeatured();
    Array.prototype.forEach.call(grid.querySelectorAll(".card"), function (c) {
      c.querySelector(".stage").className = stageClass(c.dataset.id);
    });
  }

  function restart(el) {
    el.style.animation = "none";
    void el.offsetWidth; // force reflow
    el.style.animation = "";
  }
  function replay() {
    Array.prototype.forEach.call(
      document.querySelectorAll(".stage .eyes, .stage .eye, .stage .dot"),
      restart
    );
    playCurrent(); // re-trigger the featured state's sound in sync with the replay
  }

  // --- wiring ---
  spd.addEventListener("input", function () {
    state.spd = parseFloat(spd.value);
    applySpd();
    // keep a running loop bed (A3/B1/B2/B3) in tempo with the new --spd
    if (state.sound && SE && /loop/i.test((find(state.id) || {}).type || "")) playCurrent();
  });

  document.getElementById("shape").addEventListener("click", function (e) {
    var b = e.target.closest("button[data-shape]");
    if (!b) return;
    state.shape = b.dataset.shape;
    Array.prototype.forEach.call(this.querySelectorAll("button"), function (x) {
      x.classList.toggle("on", x === b);
    });
    refreshAllStages();
  });

  document.getElementById("tint").addEventListener("click", function (e) {
    var b = e.target.closest("button[data-tint]");
    if (!b) return;
    document.body.classList.toggle("tint-cyan", b.dataset.tint === "cyan");
    Array.prototype.forEach.call(this.querySelectorAll("button"), function (x) {
      x.classList.toggle("on", x === b);
    });
  });

  document.getElementById("replay").addEventListener("click", replay);

  if (SE && SE.available()) {
    soundBtn.addEventListener("click", function () {
      state.sound = !state.sound;
      soundBtn.textContent = state.sound ? "🔊 Sound" : "🔇 Muted";
      soundBtn.setAttribute("aria-pressed", String(state.sound));
      if (state.sound) playCurrent();   // gesture → can start audio; voice current state
      else SE.stopAll();
    });
  } else {
    soundBtn.disabled = true;
    soundBtn.textContent = "🔇 n/d";
    soundBtn.title = "Web Audio non disponibile in questo browser";
  }

  // --- init ---
  applySpd();
  renderGrid();
  applyFeatured();
  tickClock();
  setInterval(tickClock, 1000);
})();
