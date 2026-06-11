/* sound.js — coherent sound design for the Maind X state set.
   Pure Web Audio synthesis: no audio files, no dependencies — fits an embedded
   device. One timbral family for all states: soft sine/triangle voices, a warm
   D-pentatonic palette, a gentle lowpass + short reverb tail. Calm by design
   (spec: never alarming, never hypnotic). Loop states (A3/B1/B2/B3) return a
   controller so the bed stops when you leave the state.

   On the real device the SAME envelope drives eyes + light + audio together;
   here each state restarts on select/replay so motion and sound stay in sync. */
window.SoundEngine = (function () {
  "use strict";

  // --- D major pentatonic family (Hz) -------------------------------------
  var N = {
    A2: 110.00, D3: 146.83, Fs3: 185.00, A3: 220.00,
    D4: 293.66, E4: 329.63, Fs4: 369.99, A4: 440.00, B4: 493.88,
    D5: 587.33, E5: 659.25, Fs5: 739.99, A5: 880.00
  };

  var ctx = null, bus = null, master = null, loop = null;

  function build() {
    var AC = window.AudioContext || window.webkitAudioContext;
    ctx = new AC();

    master = ctx.createGain();
    master.gain.value = 0.85;
    master.connect(ctx.destination);

    var lp = ctx.createBiquadFilter();      // keep everything warm, never harsh
    lp.type = "lowpass"; lp.frequency.value = 2600; lp.Q.value = 0.4;
    lp.connect(master);

    bus = ctx.createGain();                 // all voices land here
    bus.gain.value = 1;
    bus.connect(lp);

    // gentle space: a short feedback delay, low wet level
    var delay = ctx.createDelay(0.5); delay.delayTime.value = 0.13;
    var fb = ctx.createGain(); fb.gain.value = 0.22;
    var wet = ctx.createGain(); wet.gain.value = 0.10;
    bus.connect(delay); delay.connect(fb); fb.connect(delay); delay.connect(wet); wet.connect(lp);
  }

  function ensure() {
    if (!ctx) build();
    if (ctx.state === "suspended") ctx.resume();
    return ctx;
  }

  function now() { return ctx.currentTime; }

  /* one soft voice: sine fundamental (+ optional warm sub octave), gentle ADSR */
  function blip(freq, t0, dur, gain, opts) {
    opts = opts || {};
    var o = ctx.createOscillator();
    o.type = opts.type || "sine";
    o.frequency.setValueAtTime(freq, t0);
    if (opts.glideTo) o.frequency.exponentialRampToValueAtTime(opts.glideTo, t0 + dur);

    var g = ctx.createGain();
    var atk = opts.atk != null ? opts.atk : Math.min(0.04, dur * 0.25);
    g.gain.setValueAtTime(0.0001, t0);
    g.gain.exponentialRampToValueAtTime(gain, t0 + atk);
    g.gain.exponentialRampToValueAtTime(0.0001, t0 + dur);

    o.connect(g); g.connect(bus);
    o.start(t0); o.stop(t0 + dur + 0.05);

    if (opts.sub) {                          // quiet octave-down body for warmth
      var s = ctx.createOscillator(); s.type = "triangle";
      s.frequency.setValueAtTime(freq / 2, t0);
      var sg = ctx.createGain();
      sg.gain.setValueAtTime(0.0001, t0);
      sg.gain.exponentialRampToValueAtTime(gain * 0.5, t0 + atk);
      sg.gain.exponentialRampToValueAtTime(0.0001, t0 + dur);
      s.connect(sg); sg.connect(bus);
      s.start(t0); s.stop(t0 + dur + 0.05);
    }
    return { o: o, g: g };
  }

  /* a sustained pad with a slow breathing LFO on its level; returns stop() */
  function pad(freqs, centerGain, depth, periodS, t0) {
    var oscs = [], padGain = ctx.createGain();
    padGain.gain.setValueAtTime(0.0001, t0);
    padGain.gain.exponentialRampToValueAtTime(centerGain, t0 + 0.6);
    padGain.connect(bus);

    freqs.forEach(function (f) {
      var o = ctx.createOscillator(); o.type = "sine";
      o.frequency.value = f; o.connect(padGain); o.start(t0); oscs.push(o);
    });

    var lfo = ctx.createOscillator(); lfo.type = "sine";
    lfo.frequency.value = 1 / periodS;
    var lg = ctx.createGain(); lg.gain.value = depth;
    lfo.connect(lg); lg.connect(padGain.gain); lfo.start(t0); oscs.push(lfo);

    return function stop() {
      var t = now();
      try {
        padGain.gain.cancelScheduledValues(t);
        padGain.gain.setValueAtTime(padGain.gain.value, t);
        padGain.gain.exponentialRampToValueAtTime(0.0001, t + 0.35);
      } catch (e) {}
      oscs.forEach(function (o) { try { o.stop(t + 0.4); } catch (e) {} });
    };
  }

  /* schedule a repeating pattern of blips (speech cadence / thinking blips) */
  function pattern(steps, cycleS, spd, defGain, defDur, opts) {
    var cycleMs = cycleS * spd * 1000, stopped = false, timer = null;
    function fire() {
      if (stopped) return;
      var base = now() + 0.02;
      steps.forEach(function (s) {
        blip(s.f, base + s.at * cycleS * spd, (s.dur || defDur) * spd, s.g || defGain, opts);
      });
    }
    fire();
    timer = setInterval(fire, cycleMs);
    return function stop() { stopped = true; clearInterval(timer); };
  }

  // --- per-state voices ----------------------------------------------------
  // one-shots return undefined; loop states return a stop() controller.
  var VOICES = {
    // A · system & power
    A1_off: function (t, spd) {                       // soft power-down, descending
      blip(N.Fs4, t, 0.18 * spd, 0.18, { sub: true });
      blip(N.D4, t + 0.11 * spd, 0.55 * spd, 0.20, { sub: true, glideTo: N.A3 });
    },
    A2_wake: function (t, spd) {                       // waking: gentle rise
      blip(N.D4, t, 0.22 * spd, 0.18, { sub: true });
      blip(N.Fs4, t + 0.12 * spd, 0.24 * spd, 0.18, { sub: true });
      blip(N.A4, t + 0.24 * spd, 0.40 * spd, 0.20, { sub: true });
      blip(N.A5, t + 0.30 * spd, 0.50 * spd, 0.05);   // shimmer
    },
    A3_idle: function () { /* idle / waiting is silent — no sound */ },
    A4_clock: function (t, spd) {                      // LOOP: soft tick / tock, ~1 Hz
      return pattern([
        { at: 0.00, f: N.D4, g: 0.05, dur: 0.035 },
        { at: 0.50, f: N.A3, g: 0.04, dur: 0.035 }
      ], 2.0, spd, 0.05, 0.035);
    },

    // B · conversation
    B1_speaking: function (t, spd) {                   // SHORT voiced cue (not a loop)
      blip(N.Fs4, t, 0.10 * spd, 0.10, { sub: true });
      blip(N.A4, t + 0.09 * spd, 0.12 * spd, 0.10, { sub: true });
    },
    B2_listening: function (t, spd) {                  // SHORT entry chime (no sustained pad)
      blip(N.D4, t, 0.12 * spd, 0.12);
      blip(N.A4, t + 0.09 * spd, 0.16 * spd, 0.13);
    },
    B3_thinking: function (t, spd) {                   // LOOP: sparse contemplative blips
      return pattern([
        { at: 0.05, f: N.Fs4 }, { at: 0.22, f: N.E4 }, { at: 0.58, f: N.A4 }, { at: 0.74, f: N.Fs4 }
      ], 3.4, spd, 0.09, 0.16, { sub: true });
    },

    // C · feedback
    C1_confirm: function (t, spd) {                    // bright positive double note
      blip(N.A4, t, 0.12 * spd, 0.20);
      blip(N.D5, t + 0.10 * spd, 0.22 * spd, 0.22);
    },
    C2_didnt_catch: function (t, spd) {                // questioning upward "hm?"
      blip(N.E4, t, 0.16 * spd, 0.18, { sub: true });
      blip(N.A4, t + 0.16 * spd, 0.34 * spd, 0.18, { sub: true, glideTo: N.B4 });
    },

    // D · proactive
    D1_reminder: function (t, spd) {                   // calm reminder chime (bell-ish)
      blip(N.D5, t, 0.5 * spd, 0.16);
      blip(N.D5 * 2, t, 0.4 * spd, 0.03);              // soft harmonic for a bell timbre
      blip(N.Fs5, t + 0.16 * spd, 0.55 * spd, 0.14);
      blip(N.A5, t + 0.22 * spd, 0.5 * spd, 0.045);    // shimmer
    },
    D2_wakeword: function (t, spd) {                   // attentive, alert pop (rising)
      blip(N.D4, t, 0.09 * spd, 0.17, { sub: true });
      blip(N.A4, t + 0.07 * spd, 0.10 * spd, 0.19);
      blip(N.D5, t + 0.14 * spd, 0.26 * spd, 0.20);
    },

    // E · personality
    E1_dizzy: function (t, spd) {                      // playful wobble (vibrato + glissando)
      var o = ctx.createOscillator(); o.type = "triangle";
      var g = ctx.createGain();
      var d = 0.95 * spd;
      o.frequency.setValueAtTime(N.A4, t);
      o.frequency.linearRampToValueAtTime(N.D4, t + d * 0.4);
      o.frequency.linearRampToValueAtTime(N.Fs4, t + d * 0.7);
      o.frequency.linearRampToValueAtTime(N.E4, t + d);
      g.gain.setValueAtTime(0.0001, t);
      g.gain.exponentialRampToValueAtTime(0.16, t + 0.03);
      g.gain.exponentialRampToValueAtTime(0.0001, t + d);
      var lfo = ctx.createOscillator(); lfo.frequency.value = 9; // wobble
      var lg = ctx.createGain(); lg.gain.value = 22;
      lfo.connect(lg); lg.connect(o.frequency);
      o.connect(g); g.connect(bus);
      o.start(t); lfo.start(t); o.stop(t + d + 0.05); lfo.stop(t + d + 0.05);
    },
    E2_quirk: function (t, spd) {                      // curious little blips
      blip(N.A4, t, 0.14 * spd, 0.15, { sub: true });
      blip(N.Fs4, t + 0.18 * spd, 0.16 * spd, 0.15, { sub: true });
      blip(N.A4, t + 0.40 * spd, 0.20 * spd, 0.13);
    },
    E3_happy: function (t, spd) {                      // bright little giggle (rising)
      blip(N.D5, t, 0.12 * spd, 0.16);
      blip(N.Fs5, t + 0.10 * spd, 0.16 * spd, 0.15);
      blip(N.A5, t + 0.20 * spd, 0.30 * spd, 0.06);    // shimmer
    },
    E4_wink: function (t, spd) {                       // playful upward blip
      blip(N.A4, t, 0.16 * spd, 0.17, { glideTo: N.E5 });
    },
    E5_content: function (t, spd) {                    // warm, settling sigh (downward)
      blip(N.A4, t, 0.6 * spd, 0.15, { sub: true, glideTo: N.Fs4 });
      blip(N.D4, t + 0.05 * spd, 0.7 * spd, 0.10, { sub: true });
    }
  };

  function stopLoop() { if (loop) { try { loop(); } catch (e) {} loop = null; } }

  return {
    /* called on select & replay: stop any bed, voice the state, start its loop */
    enter: function (id, spd) {
      ensure();
      stopLoop();
      var fn = VOICES[id];
      if (!fn) return;
      var ctrl = fn(now() + 0.02, spd || 1);
      if (typeof ctrl === "function") loop = ctrl;
    },
    stopAll: function () { stopLoop(); },
    available: function () {
      return !!(window.AudioContext || window.webkitAudioContext);
    }
  };
})();
