/* face.js — controller for the FACE (alt) states gallery.
   Mirrors states.js: reads window.STATES / window.STATE_SECTIONS (state-anims.js)
   and builds the featured stage + sectioned grid. The only structural difference
   from the states page is that every stage carries a .face wrapper (eyes + mouth)
   so the per-state mouth animations from face-keyframes.css have something to bite. */
(function () {
  "use strict";

  var STATES = window.STATES || [];
  var SECTIONS = window.STATE_SECTIONS || [];

  var root = document.documentElement;
  var featStage = document.getElementById("featStage");
  var fname = document.getElementById("fname");
  var grid = document.getElementById("grid");
  var spd = document.getElementById("spd");
  var spdVal = document.getElementById("spdVal");

  var flight = document.getElementById("flight");
  var fsound = document.getElementById("fsound");
  var soundBtn = document.getElementById("sound");
  var SE = window.SoundEngine;

  var flowBtn = document.getElementById("flow");
  var flowbar = document.getElementById("flowbar");
  var flowphase = document.getElementById("flowphase");
  var wakeBtn = document.getElementById("wake");
  var flow = { running: false, timers: [] };

  var state = {
    id: STATES.length ? STATES[2].id : null,   // start on A3_idle
    spd: parseFloat(spd.value),
    sound: true,
  };

  function stageClass(id) { return "stage round st-" + id; }

  function playCurrent() {
    if (!SE || !state.sound || !state.id) return;
    SE.enter(state.id, state.spd);
  }

  function applySpd() {
    root.style.setProperty("--spd", state.spd);
    spdVal.textContent = "× " + state.spd.toFixed(2);
  }

  // Stage carries the face (eyes + mouth) plus the same overlays states.html uses
  // (clock, notepad, check, bell). The state classes switch them on/off.
  var STAGE_HTML =
    '<div class="face">' +
      '<div class="eyes">' +
        '<div class="eye left"><div class="dot"></div></div>' +
        '<div class="eye right"><div class="dot"></div></div>' +
      '</div>' +
      '<div class="mouth">' +
        '<svg class="smile" viewBox="0 0 200 110" aria-hidden="true"><path d="M28 22 Q 100 110 172 22"/></svg>' +
        '<span class="open"></span>' +
      '</div>' +
    '</div>' +
    '<div class="clock"><b class="hh">--</b><b class="cl">:</b><b class="mm">--</b></div>' +
    '<div class="check"><svg viewBox="0 0 100 100"><path d="M26 52 L44 70 L76 32"/></svg></div>' +
    '<div class="bell"><svg viewBox="0 0 24 24"><path d="M12 22c1.1 0 2-.9 2-2h-4c0 1.1.9 2 2 2zm6-6v-5c0-3.07-1.63-5.64-4.5-6.32V4c0-.83-.67-1.5-1.5-1.5S10.5 3.17 10.5 4v.68C7.64 5.36 6 7.92 6 11v5l-2 2v1h16v-1l-2-2z"/></svg></div>' +
    '<div class="notes">' +
      '<div class="pad">' +
        '<span class="ring r1"></span><span class="ring r2"></span><span class="ring r3"></span><span class="ring r4"></span>' +
        '<i class="ln l1"></i><i class="ln l2"></i><i class="ln l3"></i><i class="ln l4"></i>' +
      '</div>' +
      '<div class="pen"></div>' +
    '</div>';

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
          '<div class="cmeta">' + a.type + "</div>"
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
      fname.textContent = a.name;
      if (flight) flight.textContent = a.lightDesc || "—";
      if (fsound) {
        var silent = !a.soundDesc;
        fsound.textContent = silent ? "silent (no sound)" : a.soundDesc;
        fsound.parentNode.classList.toggle("silent", silent);
      }
    }
    Array.prototype.forEach.call(grid.querySelectorAll(".card"), function (c) {
      c.classList.toggle("active", c.dataset.id === state.id);
    });
  }

  function find(id) {
    for (var i = 0; i < STATES.length; i++) if (STATES[i].id === id) return STATES[i];
    return null;
  }

  function select(id) { if (flow.running) flowStop(); state.id = id; applyFeatured(); playCurrent(); idleSync(); manageSpeech(); }

  // --- Speaking: simulate an English voice talking about hormones (Web Speech API) ---
  var TTS = window.speechSynthesis || null;
  var HORMONE_LINES = [
    "Your estrogen levels are looking nice and steady today.",
    "Remember to take your progesterone this evening.",
    "Estradiol works best on a consistent daily schedule.",
    "Your testosterone is right where we want it.",
    "Cortisol is naturally highest in the morning.",
    "Don't forget to change your estrogen patch tomorrow.",
    "Balanced hormones support your mood and your energy.",
    "You're doing great staying on track with your therapy.",
    "Your thyroid hormones look well within range.",
    "A little progesterone helps you sleep better tonight."
  ];
  var speech = { timer: null };
  // pick a natural, neutral voice for a language — prefer "Koko", then enhanced/natural
  // voices, then any voice for that language (avoids the robotic system default)
  function pickVoiceLang(lang) {
    var voices = TTS && TTS.getVoices ? TTS.getVoices() : [];
    if (!voices.length) return null;
    var m = voices.filter(function (v) { return v.lang && v.lang.toLowerCase().indexOf(lang.toLowerCase()) === 0; });
    var byName = function (re, list) { return (list || m).filter(function (v) { return re.test(v.name); }); };
    return (byName(/koko/i, voices)[0] ||                       // requested voice (if installed)
            byName(/(natural|neural|enhanced|premium)/i)[0] ||  // higher-quality = less robotic
            byName(/google/i)[0] ||                             // Google voices are natural
            byName(/\b(samantha|alex|allison|ava|joanna|nicky|aria|alice|federica|luca)\b/i)[0] ||
            m[0] || null);
  }
  function speakPhrase(text, lang) {
    if (!TTS) return;
    try { TTS.cancel(); } catch (e) {}
    var u = new SpeechSynthesisUtterance(text);
    u.rate = 0.95; u.pitch = 1;                 // calm, neutral, less robotic delivery
    var v = pickVoiceLang(lang);
    if (v) { u.voice = v; u.lang = v.lang; } else { u.lang = lang; }
    return u;
  }
  function speakHormones() {
    if (!TTS || !state.sound || state.id !== "B1_speaking") return;
    var u = speakPhrase(HORMONE_LINES[Math.floor(Math.random() * HORMONE_LINES.length)], "en");
    if (!u) return;
    u.onend = function () { if (state.id === "B1_speaking" && state.sound) speech.timer = setTimeout(speakHormones, 350); };
    TTS.speak(u);
  }
  function speakDidntCatch() {
    if (!TTS || !state.sound || state.id !== "C2_didnt_catch") return;
    var u = speakPhrase("Sorry, I didn't catch that.", "en");   // talks via the mouth, in English
    if (u) TTS.speak(u);
  }
  function manageSpeech() {
    if (speech.timer) { clearTimeout(speech.timer); speech.timer = null; }
    if (!TTS) return;
    if (state.sound && !flow.running && state.id === "B1_speaking") {
      if (!TTS.speaking && !TTS.pending) speakHormones();
    } else if (state.sound && !flow.running && state.id === "C2_didnt_catch") {
      if (!TTS.speaking && !TTS.pending) speakDidntCatch();
    } else {
      try { TTS.cancel(); } catch (e) {}
    }
  }

  // --- idle: a curious face that looks around at RANDOM (never a fixed loop) ---
  // JS drives the featured stage's gaze / head-lean / mouth / blink with random
  // targets and timings; the smile shows very often but relaxes now and then.
  // Occasionally a bigger quirk (double-take) or — rarely — a wink interrupts,
  // then the random look-around resumes. (Grid thumbnails keep the CSS loop.)
  var rand = function (a, b) { return a + Math.random() * (b - a); };
  var gaze = { look: null, blink: null, smile: null, els: null };

  function gazeStop() {
    [gaze.look, gaze.blink, gaze.smile].forEach(function (t) { if (t) clearTimeout(t); });
    gaze.look = gaze.blink = gaze.smile = null;
    if (gaze.els) {
      var e = gaze.els;
      [e.eyes, e.face, e.mouth, e.smile].forEach(function (el) {
        if (el) { el.style.animation = ""; el.style.transition = ""; el.style.transform = ""; el.style.opacity = ""; }
      });
      if (e.smilePath) { e.smilePath.style.d = ""; e.smilePath.style.transition = ""; }
      e.dots.forEach(function (d) { d.style.animation = ""; d.style.transition = ""; d.style.transform = ""; });
      gaze.els = null;
    }
  }
  function gazeStart() {
    gazeStop();
    if (state.id !== "A3_idle" || flow.running) return;
    var eyes = featStage.querySelector(".eyes");
    if (!eyes) return;
    var smile = featStage.querySelector(".smile");
    gaze.els = {
      eyes: eyes,
      face: featStage.querySelector(".face"),
      mouth: featStage.querySelector(".mouth"),
      smile: smile,
      smilePath: smile && smile.querySelector("path"),
      dots: Array.prototype.slice.call(featStage.querySelectorAll(".dot"))
    };
    ["eyes", "mouth", "smile"].forEach(function (k) { if (gaze.els[k]) gaze.els[k].style.animation = "none"; });
    gaze.els.dots.forEach(function (d) { d.style.animation = "none"; });
    // the face gently breathes (CSS) — quicker than before
    if (gaze.els.face) gaze.els.face.style.animation = "idleBreath " + Math.round(2600 * state.spd) + "ms ease-in-out infinite";
    // ultra-fluid smile <-> neutral: morph the path's curve (CSS d transition, Chrome)
    if (gaze.els.smilePath) gaze.els.smilePath.style.transition = "d 480ms ease";
    gazeLook();
    gazeBlinkLoop();
    gazeSmileLoop();
  }
  function gazeLook() {
    var e = gaze.els;
    if (!e || state.id !== "A3_idle" || flow.running) return;
    var x = rand(-28, 28), y = rand(-12, 9);                          // wider look-around
    if (Math.random() < 0.28) { x = rand(-6, 6); y = rand(-4, 4); }   // sometimes back to center
    var move = rand(240, 430) * state.spd;        // saccade
    var hold = rand(450, 1300) * state.spd;       // shorter fixation → looks around more often
    var ease = "cubic-bezier(.45,0,.25,1)";
    e.eyes.style.transition = "transform " + move + "ms " + ease;
    e.eyes.style.transform = "translate(" + x.toFixed(1) + "px," + y.toFixed(1) + "px)";
    // the head leans a touch toward the gaze — applied to the eyes layer so the .face
    // is free to run its CSS breath (mouth follows too, below)
    if (e.mouth) {
      e.mouth.style.transition = "transform " + move + "ms " + ease;
      e.mouth.style.transform = "translate(" + (x * 0.4).toFixed(1) + "px," + (y * 0.2).toFixed(1) + "px)";
    }
    if (Math.random() < 0.55) setTimeout(gazeBlink, move * 0.35);     // blink on the gaze shift
    gaze.look = setTimeout(gazeLook, move + hold);
  }
  function gazeBlink() {
    if (!gaze.els) return;
    gaze.els.dots.forEach(function (d) {
      d.style.transition = "transform 80ms ease-in";
      d.style.transform = "scaleY(.08)";
      setTimeout(function () { if (gaze.els) { d.style.transition = "transform 120ms ease-out"; d.style.transform = "scaleY(1)"; } }, 90);
    });
  }
  function gazeBlinkLoop() {
    if (!gaze.els || state.id !== "A3_idle" || flow.running) return;
    gaze.blink = setTimeout(function () { gazeBlink(); gazeBlinkLoop(); }, rand(2600, 6500) * state.spd);
  }
  function gazeSmileLoop() {
    if (!gaze.els || state.id !== "A3_idle" || flow.running) return;
    var smiling = Math.random() < 0.8;            // smiles very often, but not perpetually
    // ultra-fluid morph between a smile and a calm neutral mouth: same M..Q.. path
    // structure, so the curve eases smoothly between the two and never thins out.
    if (gaze.els.smilePath) gaze.els.smilePath.style.d = smiling ? MOUTH_SMILE : MOUTH_NEUTRAL;
    gaze.smile = setTimeout(gazeSmileLoop, (smiling ? rand(2800, 5400) : rand(900, 1800)) * state.spd);
  }
  var MOUTH_SMILE = "path('M28 22 Q 100 110 172 22')";   // the smile arc (markup default)
  var MOUTH_NEUTRAL = "path('M30 33 Q 100 41 170 33')";  // calm, nearly-flat neutral mouth

  function idleEnter() { gazeStart(); idleManage(); }
  function idleExit() { gazeStop(); idleClear(); }
  function idleSync() {
    if (state.id === "A3_idle" && !flow.running) idleEnter();
    else idleExit();
  }

  // occasional quirk / wink interludes on top of the random look-around
  var idle = { timer: null };
  function idleClear() { if (idle.timer) { clearTimeout(idle.timer); idle.timer = null; } }
  function idleManage() {
    idleClear();
    if (state.id !== "A3_idle" || flow.running) return;
    var delay = rand(6000, 13000) * state.spd;
    idle.timer = setTimeout(function () {
      if (state.id !== "A3_idle" || flow.running) return;
      var wink = Math.random() < 0.18;                       // wink is the rare one
      var id = wink ? "E4_wink" : "E2_quirk";
      var win = (wink ? 1700 : 1500) * state.spd;
      gazeStop();                                            // release JS control so the CSS quirk shows
      featStage.className = stageClass(id);
      if (SE && state.sound) SE.enter(id, state.spd);
      idle.timer = setTimeout(function () {
        if (state.id === "A3_idle" && !flow.running) {
          featStage.className = stageClass("A3_idle");
          gazeStart();                                       // resume the random look-around
          idleManage();                                      // queue the next interlude
        }
      }, win);
    }, delay);
  }

  function restart(el) {
    el.style.animation = "none";
    void el.offsetWidth;
    el.style.animation = "";
  }
  function replay() {
    Array.prototype.forEach.call(
      document.querySelectorAll(".stage, .stage .eyes, .stage .eye, .stage .dot, .stage .mouth, .stage .smile, .stage .open, .stage .check path"),
      restart
    );
    playCurrent();
  }

  spd.addEventListener("input", function () {
    state.spd = parseFloat(spd.value);
    applySpd();
    if (state.sound && SE && /loop/i.test((find(state.id) || {}).type || "")) playCurrent();
  });

  document.getElementById("replay").addEventListener("click", replay);

  if (SE && SE.available()) {
    soundBtn.addEventListener("click", function () {
      state.sound = !state.sound;
      soundBtn.textContent = state.sound ? "🔊 Sound" : "🔇 Muted";
      soundBtn.setAttribute("aria-pressed", String(state.sound));
      if (state.sound) playCurrent();
      else SE.stopAll();
      manageSpeech();
    });
  } else {
    soundBtn.disabled = true;
    soundBtn.textContent = "🔇 n/d";
    soundBtn.title = "Web Audio non disponibile in questo browser";
  }

  // --- microphone: real voice-activity listening (used after the wake word) ---
  var mic = { stream: null, ctx: null, timer: null, analyser: null };
  function micStop() {
    if (mic.timer) { clearInterval(mic.timer); mic.timer = null; }
    if (mic.stream) { mic.stream.getTracks().forEach(function (t) { t.stop(); }); mic.stream = null; }
    if (mic.ctx) { try { mic.ctx.close(); } catch (e) {} mic.ctx = null; }
    mic.analyser = null;
  }
  function micListen(onSpeech, onError) {
    micStop();
    var md = navigator.mediaDevices;
    if (!md || !md.getUserMedia) { if (onError) onError(); return; }
    md.getUserMedia({ audio: true }).then(function (stream) {
      mic.stream = stream;
      var Ctx = window.AudioContext || window.webkitAudioContext;
      mic.ctx = new Ctx();
      var src = mic.ctx.createMediaStreamSource(stream);
      mic.analyser = mic.ctx.createAnalyser();
      mic.analyser.fftSize = 512;
      src.connect(mic.analyser);
      var data = new Uint8Array(mic.analyser.fftSize);
      var heard = 0;
      mic.timer = setInterval(function () {
        if (!mic.analyser) return;
        mic.analyser.getByteTimeDomainData(data);
        var sum = 0; for (var i = 0; i < data.length; i++) { var d = data[i] - 128; sum += d * d; }
        var rms = Math.sqrt(sum / data.length);
        if (rms > 7) heard++; else heard = Math.max(0, heard - 1);   // voice activity
        if (heard >= 4) { micStop(); if (onSpeech) onSpeech(); }      // sustained speech detected
      }, 60);
    }).catch(function () { if (onError) onError(); });
  }

  // --- lifecycle flow simulation (same as states.js) ---
  var IDLE_TO_CLOCK = 6000;
  var CLOCK_AUTOWAKE = 6000;

  function flowClear() { flow.timers.forEach(clearTimeout); flow.timers = []; micStop(); }
  function flowAt(ms, fn) { flow.timers.push(setTimeout(fn, ms)); }
  function flowPhase(txt) { if (flowphase) flowphase.textContent = txt; }
  function flowShow(id) { state.id = id; applyFeatured(); playCurrent(); manageSpeech(); }

  function flowStart() {
    idleExit();
    flow.running = true;
    flowBtn.textContent = "■ Stop flow";
    flowBtn.classList.add("on");
    flowbar.hidden = false;
    flowPowerOn();
  }
  function flowStop() {
    flow.running = false; flowClear();
    flowBtn.textContent = "▶ Simulate flow";
    flowBtn.classList.remove("on");
    flowbar.hidden = true;
  }
  function flowPowerOn() {
    flowPhase("Power on…"); flowShow("A1_off");
    flowAt(800, function () {
      flowPhase("Waking up"); flowShow("A2_wake");
      flowAt(1700, flowIdle);
    });
  }
  function flowIdle() {
    flowClear();
    flowShow("A3_idle");
    wakeBtn.disabled = false;
    var left = Math.round(IDLE_TO_CLOCK / 1000);
    (function tick() {
      if (!flow.running) return;
      flowPhase("Idle / waiting — sleeps to clock in " + left + " s");
      if (left-- > 0) flowAt(1000, tick);
    })();
    flowAt(IDLE_TO_CLOCK, flowClock);
  }
  function flowClock() {
    flowClear();
    flowShow("A4_clock");
    wakeBtn.disabled = false;
    flowPhase("Clock mode (5 min idle) — say “Hey Arduino”");
    flowAt(CLOCK_AUTOWAKE, function () { if (flow.running) flowWake(); });
  }
  function flowWake() {
    flowClear();
    wakeBtn.disabled = true;
    flowPhase("Wake word → listening");
    flowShow("D2_wakeword");
    flowAt(1100, function () {
      // after the wake word the device waits for someone to speak — for real, via the mic
      flowPhase("Listening… say something into the mic");
      flowShow("B2_listening");
      micListen(
        function () {                                  // heard the user speak
          if (!flow.running) return;
          flowPhase("Got it — thinking"); flowShow("B3_thinking");
          flowAt(1400, function () {
            if (!flow.running) return;
            flowPhase("Replying"); flowShow("B1_speaking");
            flowAt(4200, flowIdle);
          });
        },
        function () { flowAt(2600, flowIdle); }        // no mic → fall back to a timed wait
      );
    });
  }

  flowBtn.addEventListener("click", function () { flow.running ? flowStop() : flowStart(); });
  wakeBtn.addEventListener("click", function () { if (flow.running) flowWake(); });

  // --- init ---
  applySpd();
  renderGrid();
  applyFeatured();
  idleSync();            // start the random look-around (+ quirk interludes) on the idle home state
  tickClock();
  setInterval(tickClock, 1000);
  wakeBtn.disabled = true;
})();
