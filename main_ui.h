#include <pgmspace.h>


  // ============================================================
  // MAIN HTML
  // ============================================================

  const char INDEX_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>

  <html>

  <head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width,initial-scale=1">

    <title>GXDE LIGHT</title>

    <style>
      /* =========================================================
   LOCKED PRESET CONTROLS
   ========================================================= */

      input[type=range]:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }

      /* =========================================================
   GLOBAL
   ========================================================= */

      * {
        box-sizing: border-box;
      }

      html,
      body {
        width: 100%;
        max-width: 100%;
        overflow-x: hidden;
      }

      body {

        margin: 0;

        background:
          radial-gradient(circle at top,
            #10262b,
            #000);

        color: #00ffe1;

        font-family:
          Arial,
          Helvetica,
          sans-serif;

        padding: 15px;
      }


      /* =========================================================
   MAIN PANEL
   ========================================================= */

      .panel {

        width: min(850px,
            100%);

        margin: auto;

        background:
          rgba(0, 255, 225, .035);

        border:
          1px solid rgba(0, 255, 225, .25);

        border-radius: 22px;

        padding: 20px;

        box-shadow:
          0 0 50px rgba(0, 255, 225, .1);
      }


      /* =========================================================
   HEADER
   ========================================================= */

      h1 {

        text-align: center;

        letter-spacing: 6px;

        margin: 5px 0;
      }

      .subtitle {

        text-align: center;

        font-size: 10px;

        letter-spacing: 3px;

        opacity: .5;

        margin-bottom: 20px;
      }


      /* =========================================================
   CLOCK / MAIN STATUS
   ========================================================= */

      .clock {

        text-align: center;

        font-size:
          clamp(40px,
            10vw,
            70px);

        font-weight: bold;

        color: #00ff9c;

        text-shadow:
          0 0 20px #00ff9c55;
      }

      .state {

        text-align: center;

        margin: 10px;

        font-size: 13px;
      }

      .brightness {

        text-align: center;

        font-size: 45px;

        font-weight: bold;

        margin: 10px;
      }


      /* =========================================================
   BRIGHTNESS BAR
   ========================================================= */

      .bar {

        height: 20px;

        background: #001514;

        border-radius: 20px;

        overflow: hidden;

        border:
          1px solid #00ffe133;
      }

      .barFill {

        height: 100%;

        width: 0%;

        background:
          linear-gradient(90deg,
            #00b968,
            #00ffe1);

        transition:
          width .4s;
      }


      /* =========================================================
   CARDS
   ========================================================= */

      .card {

        background:
          rgba(0, 255, 225, .035);

        border:
          1px solid rgba(0, 255, 225, .15);

        border-radius: 15px;

        padding: 15px;

        margin-top: 15px;
      }

      .sectionTitle {

        font-size: 12px;

        letter-spacing: 2px;

        margin-bottom: 10px;

        color: #00ff9c;
      }

      .title {

        display: flex;

        justify-content: space-between;

        align-items: center;

        gap: 10px;

        margin-bottom: 10px;

        font-size: 13px;

        letter-spacing: 1px;
      }

      .value {

        color: #00ff9c;

        font-weight: bold;

        white-space: nowrap;
      }


      /* =========================================================
   INPUTS
   ========================================================= */

      input[type=time],
      input[type=number] {

        width: 100%;

        min-width: 0;

        padding: 10px;

        background: #001514;

        color: #00ffe1;

        border:
          1px solid #00ffe155;

        border-radius: 8px;
      }

      input[type=range] {

        width: 100%;

        accent-color: #00ffe1;
      }

      input,
      button {

        max-width: 100%;
      }


      /* =========================================================
   GRID
   ========================================================= */

      .grid {

        display: grid;

        grid-template-columns:
          repeat(2,
            minmax(0, 1fr));

        gap: 10px;
      }

      .grid>div {

        min-width: 0;
      }


      /* =========================================================
   BUTTONS
   ========================================================= */

      button {

        border: 0;

        border-radius: 10px;

        padding: 13px;

        min-height: 46px;

        font-weight: bold;

        cursor: pointer;

        background:
          linear-gradient(145deg,
            #00fff0,
            #00a99b);

        color: #001b18;

        transition:
          transform .15s,
          box-shadow .2s,
          opacity .2s,
          background .2s;
      }

      button:active {

        transform:
          scale(.97);
      }

      button.red {

        background:
          linear-gradient(145deg,
            #ff5555,
            #aa0000);

        color: white;
      }

      button.green {

        background:
          linear-gradient(145deg,
            #00ff9c,
            #00b968);
      }

      button.dark {

        background: #06302c;

        color: #00ffe1;
      }

      button.full {

        grid-column:
          1 / -1;

        width: 100%;
      }


      /* =========================================================
   PRESET BUTTONS
   ========================================================= */

      .presetButton {

        min-height: 52px;

        position: relative;

        opacity: .78;
      }


      /* Active preset */

      .presetButton.active {

        background:
          linear-gradient(145deg,
            #00ff9c,
            #008f5c);

        color: #001b18;

        opacity: 1;

        border:
          1px solid #00ff9c;

        box-shadow:
          0 0 0 2px rgba(0, 255, 225, .25),

          0 0 18px rgba(0, 255, 156, .45);
      }


      /* Inactive presets */

      .presetButton:not(.active) {

        opacity: .72;
      }


      /* =========================================================
   PRESET STATUS
   ========================================================= */

      .presetStatus {

        margin-top: 12px;

        padding: 11px 12px;

        border-radius: 10px;

        background:
          rgba(0, 255, 156, .08);

        border:
          1px solid rgba(0, 255, 156, .25);

        text-align: center;

        font-size: 11px;

        letter-spacing: 1px;

        color: #8affea;
      }

      .presetStatus strong {

        color: #00ff9c;

        font-size: 14px;
      }


      /* =========================================================
   GRAPH
   ========================================================= */

      canvas {

        display: block;

        width: 100%;

        height: 250px;

        background: #00100f;

        border-radius: 10px;

        border:
          1px solid #00ffe122;
      }


      /* =========================================================
   SYSTEM STATUS
   ========================================================= */

      .statusGrid {

        display: grid;

        grid-template-columns:
          repeat(2,
            minmax(0, 1fr));

        gap: 8px;

        font-size: 12px;
      }

      .statusItem {

        padding: 10px;

        background: #00ffe106;

        border-radius: 8px;

        min-width: 0;

        overflow-wrap: anywhere;
      }

      .good {

        color: #00ff9c;
      }

      .warning {

        color: #ffd166;
      }

      .danger {

        color: #ff5555;
      }


      /* =========================================================
   EVENT LOG
   ========================================================= */

      pre {

        white-space:
          pre-wrap;

        word-break:
          break-word;

        font-size: 11px;

        line-height: 1.6;

        color: #8affea;
      }


      /* =========================================================
   SMALL TEXT
   ========================================================= */

      .small {

        font-size: 11px;

        opacity: .55;

        line-height: 1.6;
      }


      /* =========================================================
   MOBILE
   ========================================================= */

      @media (max-width:600px) {

        body {

          padding: 8px;
        }


        .panel {

          width: 100%;

          padding: 12px;

          border-radius: 16px;
        }


        h1 {

          font-size: 24px;

          letter-spacing: 4px;
        }


        .subtitle {

          font-size: 8px;

          letter-spacing: 2px;
        }


        .clock {

          font-size:
            clamp(38px,
              14vw,
              60px);
        }


        .brightness {

          font-size: 38px;
        }


        .card {

          padding: 12px;

          margin-top: 12px;

          border-radius: 12px;
        }


        /*
     All form grids become
     single-column on phones.
  */

        .grid {

          grid-template-columns: 1fr;

          gap: 10px;
        }


        /*
     Every grid item uses
     the full available width.
  */

        .grid>div {

          width: 100%;

          min-width: 0;
        }


        /*
     Buttons become full width.
  */

        .grid button {

          width: 100%;

          min-height: 48px;
        }


        /*
     Boost inputs specifically.
  */

        #boostBrightness,
        #boostMinutes {

          width: 100%;

          display: block;
        }


        /*
     Make all number/time inputs
     comfortable for touch.
  */

        input[type=time],
        input[type=number] {

          width: 100%;

          min-height: 44px;

          font-size: 16px;
        }


        /*
     Sliders.
  */

        input[type=range] {

          height: 32px;
        }


        /*
     Status becomes one column
     on smaller screens.
  */

        .statusGrid {

          grid-template-columns: 1fr;
        }


        /*
     Smaller graph.
  */

        canvas {

          height: 180px;
        }


        /*
     Prevent title/value collision.
  */

        .title {

          flex-wrap: wrap;
        }


        label {

          display: block;

          margin-bottom: 5px;
        }


        /*
     Preset buttons remain
     easy to tap.
  */

        .presetButton {

          width: 100%;

          min-height: 50px;
        }

      }


      /* =========================================================
   VERY SMALL PHONES
   ========================================================= */

      @media (max-width:380px) {

        .panel {

          padding: 10px;
        }


        .card {

          padding: 10px;
        }


        .sectionTitle {

          font-size: 11px;
        }


        .small {

          font-size: 10px;
        }


        .clock {

          font-size: 38px;
        }

      }
    </style>

  </head>


  <body>

    <div class="panel">


      <!-- =====================================================
     HEADER
     ===================================================== -->

      <h1>GXDE</h1>

      <div class="subtitle">
        SMART AQUARIUM LIGHT CONTROLLER
      </div>


      <!-- =====================================================
     CLOCK
     ===================================================== -->

      <div class="clock" id="clock">

        --:--:--

      </div>


      <div class="state" id="state">

        WAITING

      </div>


      <div class="brightness" id="brightness">

        0%

      </div>


      <div class="bar">

        <div class="barFill" id="barFill">
        </div>

      </div>


      <!-- =====================================================
     SCHEDULE GRAPH
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          TODAY'S LIGHT CURVE
        </div>

        <canvas id="graph" width="800" height="250">
        </canvas>

      </div>


      <!-- =====================================================
     LIGHT SCHEDULE
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          LIGHT SCHEDULE
        </div>


        <label>
          START TIME
        </label>

        <input type="time" id="start">


        <br>


        <div class="title">

          <span>
            MAXIMUM BRIGHTNESS
          </span>

          <span class="value" id="brightnessValue">
            100%
          </span>

        </div>

        <input type="range" id="maxBrightness" min="1" max="100">


        <br>


        <div class="title">

          <span>
            DURATION
          </span>

          <span class="value" id="durationValue">
            12h 00m
          </span>

        </div>

        <input type="range" id="duration" min="1" max="1440">


        <br>


        <div class="title">

          <span>
            RAMP UP
          </span>

          <span class="value" id="rampUpValue">
            30 min
          </span>

        </div>

        <input type="range" id="rampUp" min="0" max="120">


        <br>


        <div class="title">

          <span>
            RAMP DOWN
          </span>

          <span class="value" id="rampDownValue">
            30 min
          </span>

        </div>

        <input type="range" id="rampDown" min="0" max="120">


        <br>


        <div class="small">

          Start:
          <span id="scheduleStart">
            --:--
          </span>

          <br>

          Full brightness:
          <span id="scheduleFull">
            --:--
          </span>

          <br>

          Sunset:
          <span id="scheduleSunset">
            --:--
          </span>

          <br>

          OFF:
          <span id="scheduleOff">
            --:--
          </span>

        </div>

      </div>


      <!-- =====================================================
     MIDDAY SIESTA
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          MIDDAY SIESTA
        </div>


        <label>

          <input type="checkbox" id="siestaEnabled">

          Enable midday break

        </label>

        <br>
        <br>


        <div class="grid">

          <div>

            <label>
              BREAK START
            </label>

            <input type="time" id="siestaStart">

          </div>


          <div>

            <label>
              BREAK END
            </label>

            <input type="time" id="siestaEnd">

          </div>

        </div>

      </div>


      <!-- =====================================================
     PLANT ACCLIMATION
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          PLANT ACCLIMATION
        </div>


        <label>

          <input type="checkbox" id="acclimationEnabled">

          Enable acclimation

        </label>

        <br>
        <br>


        <div class="grid">

          <div>

            <label>
              START %
            </label>

            <input type="number" id="acclimationStart" min="1" max="100">

          </div>


          <div>

            <label>
              TARGET %
            </label>

            <input type="number" id="acclimationTarget" min="1" max="100">

          </div>


          <div>

            <label>
              DAYS
            </label>

            <input type="number" id="acclimationDays" min="1" max="365">

          </div>


          <div>

            <label>
              CURRENT DAY
            </label>

            <input type="number" id="acclimationDay" min="1" max="365">

          </div>

        </div>


        <p class="small" id="acclimationInfo">
        </p>

      </div>


      <!-- =====================================================
     PRESETS
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          PRESETS
        </div>


        <div class="grid">


          <button id="presetLowButton" class="presetButton" onclick="presetLow()">

            LOW TECH

          </button>


          <button id="presetPlantedButton" class="presetButton" onclick="presetPlanted()">

            PLANTED

          </button>


          <button id="presetBrightButton" class="presetButton" onclick="presetBright()">

            BRIGHT PLANTED

          </button>


          <button id="presetCustomButton" class="presetButton dark" onclick="presetCustom()">

            CUSTOM

          </button>


        </div>


        <div class="presetStatus" id="presetStatus">

          ACTIVE PRESET:
          <strong id="activePreset">
            CUSTOM
          </strong>

        </div>

      </div>


      <!-- =====================================================
     MANUAL CONTROL
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          MANUAL CONTROL
        </div>


        <div class="title">

          <span>
            MANUAL BRIGHTNESS
          </span>

          <span class="value" id="manualValue">
            0%
          </span>

        </div>


        <input type="range" id="manualBrightness" min="0" max="100" value="0">


        <br><br>


        <div class="grid">

          <button class="green" onclick="manualApply()">

            APPLY MANUAL

          </button>


          <button class="dark" onclick="returnAuto()">

            RETURN TO AUTO

          </button>

        </div>

      </div>


      <!-- =====================================================
     TEMPORARY BOOST
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          TEMPORARY BOOST
        </div>


        <div class="grid">


          <div>

            <label>
              BOOST %
            </label>

            <input type="number" id="boostBrightness" min="1" max="100" value="100">

          </div>


          <div>

            <label>
              DURATION MINUTES
            </label>

            <input type="number" id="boostMinutes" min="1" max="180" value="15">

          </div>


        </div>


        <br>


        <button class="full" onclick="startBoost()">

          START TEMPORARY BOOST

        </button>


        <br><br>


        <button class="red full" onclick="stopBoost()">

          STOP BOOST

        </button>

      </div>


      <!-- =====================================================
     SAVE
     ===================================================== -->

      <div class="card">

        <button class="green full" onclick="saveSettings()">

          SAVE SETTINGS

        </button>


        <p id="saveMessage" class="small">
        </p>

      </div>


      <!-- =====================================================
     SYSTEM STATUS
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          SYSTEM STATUS
        </div>


        <div class="statusGrid">


          <div class="statusItem">

            TIME SOURCE<br>

            <strong id="timeSource">
              --
            </strong>

          </div>


          <div class="statusItem">

            RTC<br>

            <strong id="rtcStatus">
              --
            </strong>

          </div>


          <div class="statusItem">

            NTP<br>

            <strong id="ntpStatus">
              --
            </strong>

          </div>


          <div class="statusItem">

            WIFI<br>

            <strong id="wifiStatus">
              --
            </strong>

          </div>


          <div class="statusItem">

            RSSI<br>

            <strong id="wifiRSSI">
              --
            </strong>

          </div>


          <div class="statusItem">

            PWM<br>

            <strong id="pwmValue">
              --
            </strong>

          </div>


          <div class="statusItem">

            MODE<br>

            <strong id="mode">
              --
            </strong>

          </div>


          <div class="statusItem">

            NEXT EVENT<br>

            <strong id="nextEvent">
              --
            </strong>

          </div>


        </div>

      </div>


      <!-- =====================================================
     EVENT LOG
     ===================================================== -->

      <div class="card">

        <div class="sectionTitle">
          EVENT LOG
        </div>


        <pre id="events">
Loading...
</pre>

      </div>


      <!-- =====================================================
     FOOTER
     ===================================================== -->

      <div class="small" style="text-align:center;margin-top:20px">

        GXDE SMART AQUARIUM LIGHT CONTROLLER

      </div>


    </div>


    <script>


      // ==========================================================
      // ELEMENT HELPER
      // ==========================================================

      const $ =
        id =>
          document.getElementById(id);


      // ==========================================================
      // PRESET SYSTEM
      // ==========================================================

      let activePreset =
        "CUSTOM";


      // ==========================================================
      // FORMAT
      // ==========================================================

      function pad(n) {

        return String(n)
          .padStart(2, "0");

      }


      function formatMinutes(total) {

        total =
          (
            total % 1440 +
            1440
          ) % 1440;


        let h =
          Math.floor(
            total / 60
          );


        let m =
          total % 60;


        return (
          pad(h) +
          ":" +
          pad(m)
        );

      }


      // ==========================================================
      // UPDATE PRESET INDICATOR
      // ==========================================================

      function updatePresetIndicator() {

        const buttons = [

          "presetLowButton",

          "presetPlantedButton",

          "presetBrightButton",

          "presetCustomButton"

        ];


        buttons.forEach(
          id => {

            const button =
              $(id);

            if (button) {

              button.classList.remove(
                "active"
              );

            }

          });


        let activeButton;


        if (
          activePreset ===
          "LOW TECH"
        ) {

          activeButton =
            $("presetLowButton");

        }

        else if (
          activePreset ===
          "PLANTED"
        ) {

          activeButton =
            $("presetPlantedButton");

        }

        else if (
          activePreset ===
          "BRIGHT PLANTED"
        ) {

          activeButton =
            $("presetBrightButton");

        }

        else {

          activeButton =
            $("presetCustomButton");

        }


        if (activeButton) {

          activeButton.classList.add(
            "active"
          );

        }


        $("activePreset").innerText =
          activePreset;

        updatePresetFields();
      }

      function updatePresetFields() {

        const locked =
          activePreset !== "CUSTOM";

        const fields = [
          "duration",
          "maxBrightness",
          "rampUp",
          "rampDown"
        ];

        fields.forEach(id => {

          const field = $(id);

          if (field) {

            field.disabled = locked;

          }

        });

      }

      // ==========================================================
      // DETECT PRESET
      // ==========================================================

      function detectPreset() {

        let brightness =
          parseInt(
            $("maxBrightness").value
          );


        let rampUp =
          parseInt(
            $("rampUp").value
          );


        let rampDown =
          parseInt(
            $("rampDown").value
          );


        let duration =
          parseInt(
            $("duration").value
          );


        if (

          brightness === 50 &&

          rampUp === 30 &&

          rampDown === 30 &&

          duration === 480

        ) {

          activePreset =
            "LOW TECH";

        }


        else if (

          brightness === 70 &&

          rampUp === 30 &&

          rampDown === 30 &&

          duration === 540

        ) {

          activePreset =
            "PLANTED";

        }


        else if (

          brightness === 100 &&

          rampUp === 45 &&

          rampDown === 45 &&

          duration === 600

        ) {

          activePreset =
            "BRIGHT PLANTED";

        }


        else {

          activePreset =
            "CUSTOM";

        }


        updatePresetIndicator();

      }


      // ==========================================================
      // UPDATE DISPLAY VALUES
      // ==========================================================

      function updateValues() {

        let d =
          parseInt(
            $("duration").value
          );


        let h =
          Math.floor(
            d / 60
          );


        let m =
          d % 60;


        $("durationValue").innerText =

          h +
          "h " +
          pad(m) +
          "m";


        $("brightnessValue").innerText =

          $("maxBrightness").value +
          "%";


        $("rampUpValue").innerText =

          $("rampUp").value +
          " min";


        $("rampDownValue").innerText =

          $("rampDown").value +
          " min";


        $("manualValue").innerText =

          $("manualBrightness").value +
          "%";


        updateScheduleDisplay();

        drawGraph();

      }


      // ==========================================================
      // START TIME
      // ==========================================================

      function getStart() {

        let parts =
          $("start").value.split(":");


        if (
          parts.length !== 2
        )
          return 0;


        return (

          parseInt(parts[0]) *
          60 +

          parseInt(parts[1])

        );

      }


      // ==========================================================
      // SCHEDULE DISPLAY
      // ==========================================================

      function updateScheduleDisplay() {

        let start =
          getStart();


        let duration =
          parseInt(
            $("duration").value
          );


        let rampUp =
          parseInt(
            $("rampUp").value
          );


        let rampDown =
          parseInt(
            $("rampDown").value
          );


        $("scheduleStart").innerText =

          formatMinutes(
            start
          );


        $("scheduleFull").innerText =

          formatMinutes(
            start +
            rampUp
          );


        $("scheduleSunset").innerText =

          formatMinutes(
            start +
            duration -
            rampDown
          );


        $("scheduleOff").innerText =

          formatMinutes(
            start +
            duration
          );

      }


      // ==========================================================
      // GRAPH
      // ==========================================================

      function drawGraph() {

        let canvas =
          $("graph");


        let ctx =
          canvas.getContext("2d");


        let w =
          canvas.width;


        let h =
          canvas.height;


        ctx.clearRect(
          0,
          0,
          w,
          h
        );


        ctx.fillStyle =
          "#00100f";


        ctx.fillRect(
          0,
          0,
          w,
          h
        );


        ctx.strokeStyle =
          "#00ffe122";


        ctx.lineWidth =
          1;


        for (
          let i = 0;
          i <= 4;
          i++
        ) {

          let y =
            20 +
            (
              i *
              (
                h - 50
              ) / 4
            );


          ctx.beginPath();


          ctx.moveTo(
            35,
            y
          );


          ctx.lineTo(
            w - 10,
            y
          );


          ctx.stroke();

        }


        ctx.strokeStyle =
          "#00ffe155";


        ctx.beginPath();


        ctx.moveTo(
          35,
          10
        );


        ctx.lineTo(
          35,
          h - 30
        );


        ctx.lineTo(
          w - 10,
          h - 30
        );


        ctx.stroke();


        let start =
          getStart();


        let duration =
          parseInt(
            $("duration").value
          );


        let rampUp =
          parseInt(
            $("rampUp").value
          );


        let rampDown =
          parseInt(
            $("rampDown").value
          );


        let max =
          parseInt(
            $("maxBrightness").value
          );


        ctx.beginPath();


        for (
          let minute = 0;
          minute < 1440;
          minute += 5
        ) {

          let brightness =
            0;


          let elapsed =
            minute -
            start;


          if (
            elapsed < 0
          )
            elapsed += 1440;


          if (

            duration >= 1440 ||

            elapsed < duration

          ) {

            if (

              rampUp > 0 &&

              elapsed < rampUp

            ) {

              let p =
                elapsed /
                rampUp;


              p =
                p * p *
                (3 - 2 * p);


              brightness =
                p * max;

            }


            else if (

              rampDown > 0 &&

              duration < 1440 &&

              elapsed >=
              duration - rampDown

            ) {

              let p =

                (
                  elapsed -
                  (
                    duration -
                    rampDown
                  )
                ) /
                rampDown;


              p =
                p * p *
                (3 - 2 * p);


              brightness =
                (1 - p) * max;

            }


            else {

              brightness =
                max;

            }

          }


          let x =

            35 +
            (
              minute /
              1440
            ) *
            (
              w - 45
            );


          let y =

            h - 30 -

            (
              brightness /
              100
            ) *
            (
              h - 45
            );


          if (
            minute === 0
          )

            ctx.moveTo(
              x,
              y
            );

          else

            ctx.lineTo(
              x,
              y
            );

        }


        ctx.strokeStyle =
          "#00ff9c";


        ctx.lineWidth =
          3;


        ctx.stroke();


        ctx.fillStyle =
          "#00ffe188";


        ctx.font =
          "11px Arial";


        for (
          let hour = 0;
          hour < 24;
          hour += 3
        ) {

          let x =

            35 +
            (
              hour * 60 /
              1440
            ) *
            (
              w - 45
            );


          ctx.fillText(

            pad(hour) +
            ":00",

            x - 12,

            h - 10

          );

        }

      }

      // ==========================================================
      // LOAD SETTINGS
      // ==========================================================

      async function loadSettings() {

        try {

          let r =
            await fetch(
              "/settings"
            );


          let d =
            await r.json();


          $("start").value =

            pad(d.startHour) +
            ":" +
            pad(d.startMinute);


          $("duration").value =
            d.duration;


          $("maxBrightness").value =
            d.brightness;


          $("rampUp").value =
            d.rampUp;


          $("rampDown").value =
            d.rampDown;


          $("siestaEnabled").checked =
            d.siestaEnabled;


          $("siestaStart").value =

            pad(d.siestaStartHour) +
            ":" +
            pad(d.siestaStartMinute);


          $("siestaEnd").value =

            pad(d.siestaEndHour) +
            ":" +
            pad(d.siestaEndMinute);


          $("acclimationEnabled").checked =
            d.acclimationEnabled;


          $("acclimationStart").value =
            d.acclimationStart;


          $("acclimationTarget").value =
            d.acclimationTarget;


          $("acclimationDays").value =
            d.acclimationDays;


          $("acclimationDay").value =
            d.acclimationDay;


          /*
             Use the preset reported by the ESP8266.
             This makes the preset indicator persistent
             after page refresh/reboot.
          */

          if (d.preset) {

            activePreset =
              d.preset;

          }

          else {

            detectPreset();

          }


          updateValues();

          updatePresetIndicator();

        }

        catch (e) {

          console.log(
            "Settings load error:",
            e
          );

        }

      }



      // ==========================================================
      // STATUS
      // ==========================================================

      async function updateStatus() {

        try {

          let r =
            await fetch(
              "/status"
            );


          let d =
            await r.json();


          $("clock").innerText =
            d.time;


          $("brightness").innerText =
            d.brightness +
            "%";


          $("barFill").style.width =
            d.brightness +
            "%";


          $("state").innerText =
            d.state;


          $("timeSource").innerText =
            d.timeSource;


          $("rtcStatus").innerText =
            d.rtc;


          $("ntpStatus").innerText =
            d.ntp;


          $("wifiStatus").innerText =
            d.wifi;


          $("wifiRSSI").innerText =
            d.rssi;


          $("pwmValue").innerText =
            d.pwm;


          $("mode").innerText =
            d.mode;


          $("nextEvent").innerText =
            d.nextEvent;


          $("acclimationInfo").innerText =

            "Effective maximum today: " +
            d.effectiveMax +
            "%";

        }

        catch (e) {

          console.log(
            "Status error:",
            e
          );

        }

      }


      // ==========================================================
      // EVENTS
      // ==========================================================

      async function loadEvents() {

        try {

          let r =
            await fetch(
              "/events"
            );


          let d =
            await r.json();


          $("events").innerText =
            d.events.join("\n");

        }

        catch (e) {

          console.log(
            "Events error:",
            e
          );

        }

      }


      // ==========================================================
      // SAVE SETTINGS
      // ==========================================================

      async function saveSettings() {

        let start =
          $("start").value;


        let params =
          new URLSearchParams({

            start: start,

            duration:
              $("duration").value,

            brightness:
              $("maxBrightness").value,

            rampUp:
              $("rampUp").value,

            rampDown:
              $("rampDown").value,

            siestaEnabled:
              $("siestaEnabled").checked
                ? 1
                : 0,

            siestaStart:
              $("siestaStart").value,

            siestaEnd:
              $("siestaEnd").value,

            acclimationEnabled:
              $("acclimationEnabled").checked
                ? 1
                : 0,

            acclimationStart:
              $("acclimationStart").value,

            acclimationTarget:
              $("acclimationTarget").value,

            acclimationDays:
              $("acclimationDays").value,

            acclimationDay:
              $("acclimationDay").value,

            preset:
              activePreset

          });


        try {

          let r =
            await fetch(
              "/save?" +
              params.toString()
            );


          let d =
            await r.json();


          if (
            d.success
          ) {

            $("saveMessage").innerText =
              "SETTINGS SAVED";


            /*
               Reload the settings from
               the ESP8266 after saving.
            */

            await loadSettings();

          }

          else {

            $("saveMessage").innerText =
              "SAVE FAILED";

          }

        }

        catch (e) {

          $("saveMessage").innerText =
            "SAVE ERROR";

          console.log(
            "Save error:",
            e
          );

        }

      }


      // ==========================================================
      // MANUAL CONTROL
      // ==========================================================

      async function manualApply() {

        let value =
          $("manualBrightness").value;


        await fetch(

          "/manual?brightness=" +
          value

        );

      }


      // ==========================================================
      // RETURN TO AUTO
      // ==========================================================

      async function returnAuto() {

        await fetch(
          "/auto"
        );

      }


      // ==========================================================
      // TEMPORARY BOOST
      // ==========================================================

      async function startBoost() {

        let brightness =
          $("boostBrightness").value;


        let minutes =
          $("boostMinutes").value;


        await fetch(

          "/boost?brightness=" +
          brightness +
          "&minutes=" +
          minutes

        );

      }


      // ==========================================================
      // STOP BOOST
      // ==========================================================

      async function stopBoost() {

        await fetch(
          "/boostStop"
        );

      }


      // ==========================================================
      // PRESET: LOW TECH
      // ==========================================================

      function presetLow() {

        $("maxBrightness").value =
          50;


        $("rampUp").value =
          30;


        $("rampDown").value =
          30;


        $("duration").value =
          480;


        activePreset =
          "LOW TECH";


        updateValues();

        updatePresetIndicator();

      }


      // ==========================================================
      // PRESET: PLANTED
      // ==========================================================

      function presetPlanted() {

        $("maxBrightness").value =
          70;


        $("rampUp").value =
          30;


        $("rampDown").value =
          30;


        $("duration").value =
          540;


        activePreset =
          "PLANTED";


        updateValues();

        updatePresetIndicator();

      }


      // ==========================================================
      // PRESET: BRIGHT PLANTED
      // ==========================================================

      function presetBright() {

        $("maxBrightness").value =
          100;


        $("rampUp").value =
          45;


        $("rampDown").value =
          45;


        $("duration").value =
          600;


        activePreset =
          "BRIGHT PLANTED";


        updateValues();

        updatePresetIndicator();

      }


      // ==========================================================
      // PRESET: CUSTOM
      // ==========================================================
      function presetCustom() {

        activePreset =
          "CUSTOM";

        updatePresetIndicator();

      }


      // ==========================================================
      // SLIDER EVENTS
      // ==========================================================

      [
        "duration",
        "maxBrightness",
        "rampUp",
        "rampDown",
        "manualBrightness"
      ]
        .forEach(
          id => {

            $(id).addEventListener(
              "input",
              function () {

                updateValues();


                /*
                   Only schedule-related
                   controls affect preset
                   detection.
            
                   Manual brightness does
                   NOT change the preset.
                */

                if (

                  id === "duration" ||

                  id === "maxBrightness" ||

                  id === "rampUp" ||

                  id === "rampDown"

                ) {

                  detectPreset();

                }

              }
            );

          });


      // ==========================================================
      // INITIALIZATION
      // ==========================================================

      async function init() {

        await loadSettings();

        await updateStatus();

        await loadEvents();

        drawGraph();

      }


      init();


      // ==========================================================
      // LIVE STATUS REFRESH
      // ==========================================================

      setInterval(
        updateStatus,
        1000
      );


      setInterval(
        loadEvents,
        5000
      );

    </script>

  </body>

  </html>

  )rawliteral";