/* eyes.js — preview gallery controller.
   Reads window.ANIMS (anims.js), builds the featured stage + grid, handles
   speed / screen shape / blink / replay. No dependencies. */
(function () {
  "use strict";

  var ANIMS = window.ANIMS || [];
  var APPEAR_FRACTION = 0.16; // the appear takes ~16% of the cycle (settles at 16% in the keyframes)

  var root = document.documentElement;
  var featStage = document.getElementById("featStage");
  var fname = document.getElementById("fname");
  var fdesc = document.getElementById("fdesc");
  var grid = document.getElementById("grid");
  var speed = document.getElementById("speed");
  var speedVal = document.getElementById("speedVal");

  var state = {
    id: ANIMS.length ? ANIMS[0].id : null,
    appearMs: parseInt(speed.value, 10),
    blink: true,
  };

  // Round-only (the device screen is round).
  function stageClass(id) {
    return "stage round anim-" + id + (state.blink ? " blink" : "");
  }

  function applyLoop() {
    var loop = Math.round(state.appearMs / APPEAR_FRACTION);
    root.style.setProperty("--loop", loop + "ms");
    speedVal.textContent = state.appearMs + " ms · cycle " + (loop / 1000).toFixed(1) + " s";
  }

  function eyesEl() {
    var e = document.createElement("div");
    e.className = "eyes";
    e.innerHTML = '<div class="eye left"></div><div class="eye right"></div>';
    return e;
  }

  function renderGrid() {
    grid.innerHTML = "";
    ANIMS.forEach(function (a) {
      var card = document.createElement("div");
      card.className = "card" + (a.id === state.id ? " active" : "");
      card.dataset.id = a.id;

      var thumb = document.createElement("div");
      thumb.className = "thumb";
      var stage = document.createElement("div");
      stage.className = stageClass(a.id);
      stage.appendChild(eyesEl());
      thumb.appendChild(stage);

      card.appendChild(thumb);
      card.insertAdjacentHTML(
        "beforeend",
        '<div class="cname">' + a.name + "</div>" +
        '<div class="cmeta">' + a.id + " · ~" + a.appearMs + " ms</div>"
      );
      card.addEventListener("click", function () { select(a.id); });
      grid.appendChild(card);
    });
  }

  function applyFeatured() {
    if (!state.id) return;
    featStage.className = stageClass(state.id);
    var a = find(state.id);
    if (a) { fname.textContent = a.name; fdesc.textContent = a.desc; }
    Array.prototype.forEach.call(grid.querySelectorAll(".card"), function (c) {
      c.classList.toggle("active", c.dataset.id === state.id);
    });
  }

  function find(id) {
    for (var i = 0; i < ANIMS.length; i++) if (ANIMS[i].id === id) return ANIMS[i];
    return null;
  }

  function select(id) { state.id = id; applyFeatured(); }

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
      document.querySelectorAll(".stage .eye, .stage.blink .eyes"),
      restart
    );
  }

  // --- wiring ---
  speed.addEventListener("input", function () {
    state.appearMs = parseInt(speed.value, 10);
    applyLoop();
  });

  document.getElementById("blink").addEventListener("click", function (e) {
    var b = e.target.closest("button[data-blink]");
    if (!b) return;
    state.blink = b.dataset.blink === "on";
    Array.prototype.forEach.call(this.querySelectorAll("button"), function (x) {
      x.classList.toggle("on", x === b);
    });
    refreshAllStages();
  });

  document.getElementById("replay").addEventListener("click", replay);

  // --- init ---
  applyLoop();
  renderGrid();
  applyFeatured();
})();
