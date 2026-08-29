/*
   GXDE LIGHT CONTROLLER
   ESP8266 / NodeMCU V3

   ============================================================
   FEATURES
   ============================================================

   - NTP real-time clock
   - Philippines UTC+8
   - User-defined start time
   - User-defined ON duration
   - Ramp up / ramp down
   - Maximum brightness
   - Lighting settings saved to LittleFS
   - Wi-Fi credentials saved separately to LittleFS
   - Automatic recovery after power interruption
   - Responsive web UI
   - Manual ON/OFF
   - Schedule preview
   - Save confirmation
   - Built-in Wi-Fi provisioning AP
   - FLASH button Wi-Fi credential reset

   ============================================================
   WIFI SYSTEM
   ============================================================

   Normal operation:

       Saved Wi-Fi credentials
              |
              v
       Connect to Wi-Fi
              |
              v
       GXDE web interface


   If no Wi-Fi credentials exist:

       ESP starts:
       GXDE-LIGHT

       AP password:
       12345678

       Configuration page:
       http://192.168.4.1


   If saved Wi-Fi cannot connect:

       ESP automatically starts:
       GXDE-LIGHT


   ============================================================
   FLASH BUTTON
   ============================================================

   NodeMCU FLASH button = GPIO0

   Hold FLASH for 5 seconds:

       1. Delete /wifi.txt
       2. Keep /settings.txt
       3. Wait for button release
       4. Restart ESP8266
       5. Next boot starts GXDE-LIGHT AP


   IMPORTANT:

   GPIO0 is also the ESP8266 boot strap pin.

   Release FLASH before the ESP restarts.


   ============================================================
   FILE SYSTEM
   ============================================================

   /settings.txt

       Lighting settings


   /wifi.txt

       Wi-Fi SSID
       Wi-Fi password


   Wi-Fi reset NEVER deletes lighting settings.
*/


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <time.h>


// ============================================================
// HARDWARE
// ============================================================

#define PWM_PIN D5

// NodeMCU FLASH button
#define WIFI_BUTTON_PIN 0

#define PWM_MAX 1023


// ============================================================
// WIFI
// ============================================================

const char* AP_NAME =
  "GXDE-LIGHT";

const char* AP_PASSWORD =
  "12345678";


// ============================================================
// WEB SERVER
// ============================================================

ESP8266WebServer server(80);


// ============================================================
// SETTINGS
// ============================================================

struct Settings {

  int startHour;
  int startMinute;

  int durationMinutes;

  int rampUpMinutes;
  int rampDownMinutes;

  int maxBrightness;
};

Settings settings;


// ============================================================
// WIFI CREDENTIALS
// ============================================================

String wifiSSID = "";
String wifiPassword = "";

bool apMode = false;


// ============================================================
// RUNTIME
// ============================================================

bool manualMode = false;
bool manualState = false;

int currentBrightness = 0;

unsigned long lastUpdate = 0;


// ============================================================
// FLASH BUTTON
// ============================================================

bool wifiButtonArmed = false;
bool wifiButtonPrevious = HIGH;

unsigned long wifiButtonPressStart = 0;

bool wifiResetTriggered = false;

const unsigned long WIFI_BUTTON_DEBOUNCE_MS =
  50;

const unsigned long WIFI_BUTTON_HOLD_MS =
  5000;


// ============================================================
// DEFAULT LIGHT SETTINGS
// ============================================================

void defaultSettings() {

  settings.startHour = 18;
  settings.startMinute = 0;

  settings.durationMinutes =
    12 * 60;

  settings.rampUpMinutes = 10;
  settings.rampDownMinutes = 10;

  settings.maxBrightness = 100;
}


// ============================================================
// SAVE LIGHT SETTINGS
// ============================================================

bool saveSettings() {

  File file =
    LittleFS.open(
      "/settings.txt",
      "w"
    );

  if (!file) {

    Serial.println(
      "ERROR: Cannot open settings.txt"
    );

    return false;
  }

  file.println(
    settings.startHour
  );

  file.println(
    settings.startMinute
  );

  file.println(
    settings.durationMinutes
  );

  file.println(
    settings.rampUpMinutes
  );

  file.println(
    settings.rampDownMinutes
  );

  file.println(
    settings.maxBrightness
  );

  file.close();

  Serial.println(
    "Lighting settings saved"
  );

  return true;
}


// ============================================================
// LOAD LIGHT SETTINGS
// ============================================================

bool loadSettings() {

  if (
    !LittleFS.exists(
      "/settings.txt"
    )
  ) {

    Serial.println(
      "No lighting settings found"
    );

    Serial.println(
      "Using defaults"
    );

    defaultSettings();

    saveSettings();

    return false;
  }


  File file =
    LittleFS.open(
      "/settings.txt",
      "r"
    );

  if (!file) {

    Serial.println(
      "ERROR: Cannot read settings"
    );

    defaultSettings();

    return false;
  }


  settings.startHour =
    file.readStringUntil('\n')
      .toInt();

  settings.startMinute =
    file.readStringUntil('\n')
      .toInt();

  settings.durationMinutes =
    file.readStringUntil('\n')
      .toInt();

  settings.rampUpMinutes =
    file.readStringUntil('\n')
      .toInt();

  settings.rampDownMinutes =
    file.readStringUntil('\n')
      .toInt();

  settings.maxBrightness =
    file.readStringUntil('\n')
      .toInt();

  file.close();


  // ----------------------------------------------------------
  // Validate
  // ----------------------------------------------------------

  settings.startHour =
    constrain(
      settings.startHour,
      0,
      23
    );

  settings.startMinute =
    constrain(
      settings.startMinute,
      0,
      59
    );

  settings.durationMinutes =
    constrain(
      settings.durationMinutes,
      1,
      1440
    );

  settings.rampUpMinutes =
    constrain(
      settings.rampUpMinutes,
      0,
      120
    );

  settings.rampDownMinutes =
    constrain(
      settings.rampDownMinutes,
      0,
      120
    );

  settings.maxBrightness =
    constrain(
      settings.maxBrightness,
      1,
      100
    );


  Serial.println(
    "Lighting settings loaded"
  );

  return true;
}


// ============================================================
// SAVE WIFI CREDENTIALS
// ============================================================

bool saveWiFiCredentials(
  String ssid,
  String password
) {

  File file =
    LittleFS.open(
      "/wifi.txt",
      "w"
    );

  if (!file) {

    Serial.println(
      "ERROR: Cannot save WiFi credentials"
    );

    return false;
  }


  file.println(
    ssid
  );

  file.println(
    password
  );

  file.close();


  wifiSSID =
    ssid;

  wifiPassword =
    password;


  Serial.println(
    "WiFi credentials saved"
  );

  return true;
}


// ============================================================
// LOAD WIFI CREDENTIALS
// ============================================================

bool loadWiFiCredentials() {

  if (
    !LittleFS.exists(
      "/wifi.txt"
    )
  ) {

    Serial.println(
      "No saved WiFi credentials"
    );

    wifiSSID = "";
    wifiPassword = "";

    return false;
  }


  File file =
    LittleFS.open(
      "/wifi.txt",
      "r"
    );

  if (!file) {

    Serial.println(
      "ERROR: Cannot read wifi.txt"
    );

    wifiSSID = "";
    wifiPassword = "";

    return false;
  }


  wifiSSID =
    file.readStringUntil('\n');

  wifiPassword =
    file.readStringUntil('\n');

  file.close();


  wifiSSID.trim();
  wifiPassword.trim();


  if (
    wifiSSID.length() == 0
  ) {

    wifiSSID = "";
    wifiPassword = "";

    return false;
  }


  Serial.print(
    "Saved WiFi SSID: "
  );

  Serial.println(
    wifiSSID
  );

  return true;
}


// ============================================================
// DELETE WIFI CREDENTIALS
// ============================================================

void deleteWiFiCredentials() {

  Serial.println();
  Serial.println(
    "===================================="
  );

  Serial.println(
    "ERASING WIFI CREDENTIALS"
  );

  if (
    LittleFS.exists(
      "/wifi.txt"
    )
  ) {

    if (
      LittleFS.remove(
        "/wifi.txt"
      )
    ) {

      Serial.println(
        "wifi.txt deleted"
      );

    } else {

      Serial.println(
        "ERROR: Failed to delete wifi.txt"
      );
    }

  } else {

    Serial.println(
      "No wifi.txt found"
    );
  }


  wifiSSID = "";
  wifiPassword = "";


  Serial.println(
    "Lighting settings were NOT changed."
  );

  Serial.println(
    "===================================="
  );
}


// ============================================================
// PWM
// ============================================================

void setBrightness(
  int percent
) {

  percent =
    constrain(
      percent,
      0,
      100
    );

  currentBrightness =
    percent;


  int pwm =
    map(
      percent,
      0,
      100,
      0,
      PWM_MAX
    );


  analogWrite(
    PWM_PIN,
    pwm
  );
}


// ============================================================
// TIME VALIDATION
// ============================================================

bool isTimeValid() {

  time_t now =
    time(nullptr);

  return
    now > 1577836800;
}


// ============================================================
// MINUTES SINCE MIDNIGHT
// ============================================================

int minutesSinceMidnight() {

  if (
    !isTimeValid()
  )
    return -1;


  time_t now =
    time(nullptr);


  struct tm* t =
    localtime(
      &now
    );


  if (!t)
    return -1;


  return
    t->tm_hour * 60 +
    t->tm_min;
}


// ============================================================
// CURRENT TIME
// ============================================================

String getTimeString() {

  if (
    !isTimeValid()
  )
    return "--:--:--";


  time_t now =
    time(nullptr);


  struct tm* t =
    localtime(
      &now
    );


  if (!t)
    return "--:--:--";


  char buffer[20];


  sprintf(
    buffer,
    "%02d:%02d:%02d",
    t->tm_hour,
    t->tm_min,
    t->tm_sec
  );


  return String(
    buffer
  );
}


// ============================================================
// CALCULATE BRIGHTNESS
// ============================================================

int calculateBrightness() {

  int now =
    minutesSinceMidnight();


  if (
    now < 0
  )
    return 0;


  int start =
    settings.startHour * 60 +
    settings.startMinute;


  int duration =
    settings.durationMinutes;


  int rampUp =
    settings.rampUpMinutes;


  int rampDown =
    settings.rampDownMinutes;


  int elapsed =
    (
      now -
      start +
      1440
    ) % 1440;


  // ----------------------------------------------------------
  // Outside schedule
  // ----------------------------------------------------------

  if (
    duration < 1440
  ) {

    if (
      elapsed >= duration
    )
      return 0;
  }


  // ----------------------------------------------------------
  // Ramp UP
  // ----------------------------------------------------------

  if (
    rampUp > 0 &&
    elapsed < rampUp
  ) {

    float level =
      (float)elapsed /
      (float)rampUp;


    return constrain(
      (int)(
        level *
        settings.maxBrightness
      ),
      0,
      settings.maxBrightness
    );
  }


  // ----------------------------------------------------------
  // Ramp DOWN
  // ----------------------------------------------------------

  int rampDownStart =
    duration -
    rampDown;


  if (
    rampDown > 0 &&
    duration < 1440 &&
    elapsed >= rampDownStart
  ) {

    float remaining =
      (float)(
        duration -
        elapsed
      ) /
      (float)rampDown;


    return constrain(
      (int)(
        remaining *
        settings.maxBrightness
      ),
      0,
      settings.maxBrightness
    );
  }


  // ----------------------------------------------------------
  // FULL BRIGHTNESS
  // ----------------------------------------------------------

  return settings.maxBrightness;
}


// ============================================================
// UPDATE LIGHT
// ============================================================

void updateLight() {

  if (
    manualMode
  ) {

    if (
      manualState
    ) {

      setBrightness(
        settings.maxBrightness
      );

    } else {

      setBrightness(
        0
      );
    }

    return;
  }


  int brightness =
    calculateBrightness();


  setBrightness(
    brightness
  );
}


// ============================================================
// FORMAT MINUTES
// ============================================================

String formatMinutes(
  int totalMinutes
) {

  totalMinutes =
    (
      totalMinutes % 1440 +
      1440
    ) % 1440;


  int hour =
    totalMinutes / 60;


  int minute =
    totalMinutes % 60;


  char buffer[6];


  sprintf(
    buffer,
    "%02d:%02d",
    hour,
    minute
  );


  return String(
    buffer
  );
}


// ============================================================
// SCHEDULE START
// ============================================================

String getScheduleStart() {

  int start =
    settings.startHour * 60 +
    settings.startMinute;


  return formatMinutes(
    start
  );
}


// ============================================================
// FULL BRIGHTNESS TIME
// ============================================================

String getScheduleFullBrightness() {

  int start =
    settings.startHour * 60 +
    settings.startMinute;


  int full =
    start +
    settings.rampUpMinutes;


  return formatMinutes(
    full
  );
}


// ============================================================
// OFF TIME
// ============================================================

String getScheduleOff() {

  int start =
    settings.startHour * 60 +
    settings.startMinute;


  int off =
    start +
    settings.durationMinutes;


  return formatMinutes(
    off
  );
}


// ============================================================
// WIFI AP
// ============================================================

void startAccessPoint() {

  Serial.println();
  Serial.println(
    "===================================="
  );

  Serial.println(
    "STARTING GXDE WIFI SETUP"
  );


  apMode = true;


  WiFi.disconnect();


  delay(100);


  WiFi.mode(
    WIFI_AP
  );


  bool result =
    WiFi.softAP(
      AP_NAME
    );

// WiFi.softAP(
//       AP_NAME,
//       AP_PASSWORD
//     );

  if (result) {

    Serial.println(
      "Configuration AP started"
    );

    Serial.print(
      "AP name: "
    );

    Serial.println(
      AP_NAME
    );

    Serial.print(
      "AP password: "
    );

    Serial.println(
      AP_PASSWORD
    );

    Serial.print(
      "AP IP: "
    );

    Serial.println(
      WiFi.softAPIP()
    );

  } else {

    Serial.println(
      "ERROR: Failed to start AP"
    );
  }


  Serial.println(
    "===================================="
  );
}


// ============================================================
// CONNECT TO SAVED WIFI
// ============================================================

bool connectToWiFi() {

  if (
    wifiSSID.length() == 0
  ) {

    Serial.println(
      "No WiFi credentials."
    );

    return false;
  }


  Serial.println();
  Serial.println(
    "Connecting to saved WiFi..."
  );


  Serial.print(
    "SSID: "
  );

  Serial.println(
    wifiSSID
  );


  apMode = false;


  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(
    wifiSSID.c_str(),
    wifiPassword.c_str()
  );


  unsigned long start =
    millis();


  // ----------------------------------------------------------
  // Short connection timeout.
  //
  // We deliberately don't wait 20-30 seconds.
  // If connection fails, AP starts immediately.
  // ----------------------------------------------------------

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - start < 8000
  ) {

    delay(100);

    yield();
  }


  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    Serial.println();

    Serial.println(
      "WiFi connected!"
    );


    Serial.print(
      "IP: "
    );

    Serial.println(
      WiFi.localIP()
    );


    Serial.print(
      "RSSI: "
    );

    Serial.println(
      WiFi.RSSI()
    );


    return true;
  }


  Serial.println();

  Serial.println(
    "WiFi connection failed."
  );

  return false;
}


// ============================================================
// WIFI SETUP PAGE
// ============================================================

const char WIFI_HTML[] PROGMEM =
R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>GXDE WIFI</title>

<style>

*{
box-sizing:border-box;
}

body{

margin:0;

min-height:100vh;

background:
radial-gradient(
circle at top,
#0f2027,
#000
);

font-family:
Arial,
Helvetica,
sans-serif;

color:#00ffe1;

display:flex;

justify-content:center;

align-items:center;

padding:15px;
}

.panel{

width:min(
500px,
100%
);

background:
rgba(0,255,225,.05);

border:
1px solid
rgba(0,255,225,.3);

border-radius:22px;

padding:25px;

box-shadow:
0 0 40px
rgba(0,255,225,.15);
}

h1{

text-align:center;

margin:
0 0 6px;

letter-spacing:4px;
}

.subtitle{

text-align:center;

opacity:.5;

font-size:11px;

letter-spacing:2px;

margin-bottom:25px;
}

.info{

text-align:center;

font-size:13px;

line-height:1.7;

margin-bottom:20px;

opacity:.8;
}

.card{

background:
rgba(0,255,225,.04);

border:
1px solid
rgba(0,255,225,.18);

border-radius:15px;

padding:15px;

margin-bottom:13px;
}

label{

display:block;

font-size:12px;

letter-spacing:1px;

margin-bottom:8px;
}

input,
select{

width:100%;

padding:13px;

border-radius:9px;

border:
1px solid
rgba(0,255,225,.3);

background:#001514;

color:#00ffe1;

font-size:16px;
}

button{

width:100%;

border:0;

border-radius:12px;

padding:15px;

font-weight:bold;

cursor:pointer;

color:#001b18;

background:
linear-gradient(
145deg,
#00ff9c,
#00b968
);

font-size:15px;
}

.network{

margin-top:8px;

font-size:11px;

opacity:.5;

text-align:center;
}

#message{

text-align:center;

margin-top:15px;

font-size:12px;

color:#00ff9c;
}


.passwordRow {
  display: flex;
  align-items: center;
  gap: 0;
  margin-top: 7px;
}

.passwordRow input {
  flex: 1;
  margin-top: 0;
  border-radius: 6px 0 0 6px;
}

.passwordToggle {
  width: 42px;
  height: 50px;
  min-height: 37px;
  margin-top: 0;
  padding: 0;

  border-left: none;
  border-radius: 0 6px 6px 0;

  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
}
.passwordToggle svg {
  width: 20px;
  height: 20px;
}

</style>

</head>

<body>

<div class="panel">

<h1>GXDE</h1>

<div class="subtitle">
WIFI CONFIGURATION
</div>

<div class="info">

Connect GXDE to your home Wi-Fi.

<br>

The lighting settings stored inside
the controller will remain unchanged.

</div>


<form
action="/wifi/save"
method="POST">


<div class="card">

<label>
WIFI NETWORK
</label>

<select
name="ssid"
id="ssid"
required>

<option value="">
Select Wi-Fi network
</option>

</select>

<div class="network">
Or type the SSID manually below
</div>

<input
type="text"
name="manualssid"
id="manualssid"
placeholder="Wi-Fi SSID"
style="margin-top:10px">

</div>


<div class="card">

<label>
WIFI PASSWORD
</label>
<div class="passwordRow">
<input
type="password"
name="password"
id="password"
placeholder="Wi-Fi password">

     <button type="button" id="passwordButton" class="passwordToggle"
                      aria-label="Toggle password visibility" onclick="togglePassword()">
                      <!-- SVG Eye Icon (Visible by Default) -->
                      <svg id="eyeIcon" viewBox="0 0 24 24">
                        <path
                          d="M12 7c2.76 0 5 2.24 5 5 0 .65-.13 1.26-.36 1.82l2.92 2.92c1.51-1.26 2.7-2.89 3.44-4.74-1.73-4.39-6-7.5-11-7.5-1.4 0-2.74.25-3.98.7l2.16 2.16C10.74 7.13 11.35 7 12 7zM2 4.27l2.28 2.28.46.46C3.08 8.3 1.78 10.02 1 12c1.73 4.39 6 7.5 11 7.5 1.55 0 3.03-.3 4.38-.84l.42.42L19.73 22 21 20.73 3.27 3 2 4.27zM7.53 9.8l1.55 1.55c-.05.21-.08.43-.08.65 0 1.66 1.34 3 3 3 .22 0 .44-.03.65-.08l1.55 1.55c-.67.33-1.41.53-2.2.53-2.76 0-5-2.24-5-5 0-.79.2-1.53.53-2.2zm4.31-.78l3.15 3.15.01-.16c0-1.66-1.34-3-3-3l-.16.01z" />
                      </svg>
                    </button>
</div>
</div>


<button
type="submit">

SAVE WIFI & RESTART

</button>


</form>


<div id="message"></div>

</div>


<script>

   /* ========================================================
             PASSWORD SHOW / HIDE
          ======================================================== */
          const eyeOpenPath = "M12 4.5C7 4.5 2.73 7.61 1 12c1.73 4.39 6 7.5 11 7.5s9.27-3.11 11-7.5c-1.73-4.39-6-7.5-11-7.5zM12 17c-2.76 0-5-2.24-5-5s2.24-5 5-5 5 2.24 5 5-2.24 5-5 5zm0-8c-1.66 0-3 1.34-3 3s1.34 3 3 3 3-1.34 3-3-1.34-3-3-3z";
          const eyeClosedPath = "M12 7c2.76 0 5 2.24 5 5 0 .65-.13 1.26-.36 1.82l2.92 2.92c1.51-1.26 2.7-2.89 3.44-4.74-1.73-4.39-6-7.5-11-7.5-1.4 0-2.74.25-3.98.7l2.16 2.16C10.74 7.13 11.35 7 12 7zM2 4.27l2.28 2.28.46.46C3.08 8.3 1.78 10.02 1 12c1.73 4.39 6 7.5 11 7.5 1.55 0 3.03-.3 4.38-.84l.42.42L19.73 22 21 20.73 3.27 3 2 4.27zM7.53 9.8l1.55 1.55c-.05.21-.08.43-.08.65 0 1.66 1.34 3 3 3 .22 0 .44-.03.65-.08l1.55 1.55c-.67.33-1.41.53-2.2.53-2.76 0-5-2.24-5-5 0-.79.2-1.53.53-2.2zm4.31-.78l3.15 3.15.01-.16c0-1.66-1.34-3-3-3l-.16.01z";

          function togglePassword() {

            const password =
              document.getElementById(
                'password'
              );

            const button =
              document.getElementById(
                'passwordButton'
              );

            if (
              password.type === 'password'
            ) {

              password.type =
                'text';

              // button.innerText ='HIDE';
              document.querySelector('#eyeIcon path').setAttribute('d', eyeOpenPath);
            }

            else {

              password.type =
                'password';

              // button.innerText = 'SHOW';
              document.querySelector('#eyeIcon path').setAttribute('d', eyeClosedPath);
            }

          }

async function scanNetworks(){

  try{

    let r =
      await fetch(
        "/wifi/scan"
      );

    let networks =
      await r.json();

    let select =
      document.getElementById(
        "ssid"
      );


    networks.forEach(
      function(network){

        let option =
          document.createElement(
            "option"
          );

        option.value =
          network.ssid;

        option.text =
          network.ssid +
          " (" +
          network.rssi +
          " dBm)";

        select.appendChild(
          option
        );

      }
    );


    select.addEventListener(
      "change",
      function(){

        document.getElementById(
          "manualssid"
        ).value =
          this.value;

      }
    );


  }catch(e){

    console.log(e);

  }

}


scanNetworks();

</script>

</body>

</html>





)rawliteral";


// ============================================================
// NORMAL LIGHTING HTML
// ============================================================

const char INDEX_HTML[] PROGMEM =

R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>GXDE LIGHT</title>

<style>

*{
box-sizing:border-box;
}

body{

margin:0;
min-height:100vh;

background:
radial-gradient(
circle at top,
#0f2027,
#000
);

font-family:
Arial,
Helvetica,
sans-serif;

color:#00ffe1;

display:flex;
justify-content:center;
align-items:center;

padding:15px;
}

.panel{

width:min(
700px,
100%
);

background:
rgba(0,255,225,.05);

border:
1px solid
rgba(0,255,225,.3);

border-radius:22px;

padding:22px;

box-shadow:
0 0 40px
rgba(0,255,225,.15);
}

h1{

text-align:center;

margin:
0 0 6px;

letter-spacing:4px;

font-size:
clamp(
24px,
6vw,
34px
);
}

.subtitle{

text-align:center;

opacity:.5;

font-size:11px;

letter-spacing:2px;

margin-bottom:25px;
}

.clock{

text-align:center;

font-size:
clamp(
34px,
10vw,
55px
);

font-weight:bold;

color:#00ff9c;

text-shadow:
0 0 15px
rgba(0,255,156,.5);

margin-bottom:20px;
}

.status{

display:flex;

justify-content:center;

align-items:center;

gap:8px;

margin-bottom:20px;
}

.dot{

width:10px;
height:10px;

border-radius:50%;

background:#00ff9c;

box-shadow:
0 0 12px #00ff9c;
}

.card{

background:
rgba(0,255,225,.04);

border:
1px solid
rgba(0,255,225,.18);

border-radius:15px;

padding:15px;

margin-bottom:13px;
}

.title{

display:flex;

justify-content:
space-between;

margin-bottom:10px;

font-size:13px;

letter-spacing:1px;
}

.value{

color:#00ff9c;

font-weight:bold;
}

input{

width:100%;
}

input[type=time]{

background:#001514;

color:#00ffe1;

border:
1px solid
rgba(0,255,225,.3);

border-radius:8px;

padding:10px;

font-size:18px;
}

input[type=range]{

accent-color:#00ffe1;
}

.buttons{

display:grid;

grid-template-columns:
1fr 1fr;

gap:10px;
}

button{

border:0;

border-radius:12px;

padding:15px;

font-weight:bold;

cursor:pointer;

color:#001b18;

background:
linear-gradient(
145deg,
#00fff0,
#00a99b
);
}

button.off{

background:
linear-gradient(
145deg,
#ff5555,
#aa0000
);

color:white;
}

button.save{

grid-column:
1 / -1;

background:
linear-gradient(
145deg,
#00ff9c,
#00b968
);
}

.schedule{

text-align:center;

line-height:1.8;

font-size:13px;

opacity:.8;
}

.schedule span{

color:#00ff9c;

font-weight:bold;
}

.saveStatus{

text-align:center;

font-size:11px;

letter-spacing:1px;

margin-top:10px;

height:15px;

color:#00ff9c;

opacity:0;

transition:
opacity .3s;
}

.saveStatus.show{

opacity:1;
}

.footer{

text-align:center;

font-size:10px;

opacity:.4;

margin-top:15px;

letter-spacing:1px;
}

</style>

</head>

<body>

<div class="panel">

<h1>GXDE</h1>

<div class="subtitle">
SMART LIGHT CONTROLLER
</div>

<div class="clock"
id="clock">
--:--:--
</div>

<div class="status">

<div class="dot"></div>

<span id="status">
LIGHT OFF
</span>

</div>


<div class="card">

<div class="title">

<span>START TIME</span>

</div>

<input
type="time"
id="start">

</div>


<div class="card">

<div class="title">

<span>LIGHT DURATION</span>

<span
class="value"
id="durationValue">
12h 00m
</span>

</div>

<input
type="range"
id="duration"
min="1"
max="1440"
value="720">

</div>


<div class="card">

<div class="title">

<span>MAX BRIGHTNESS</span>

<span
class="value"
id="brightnessValue">
100%
</span>

</div>

<input
type="range"
id="brightness"
min="1"
max="100"
value="100">

</div>


<div class="card">

<div class="title">

<span>RAMP UP</span>

<span
class="value"
id="rampUpValue">
10 min
</span>

</div>

<input
type="range"
id="rampUp"
min="0"
max="120"
value="10">

</div>


<div class="card">

<div class="title">

<span>RAMP DOWN</span>

<span
class="value"
id="rampDownValue">
10 min
</span>

</div>

<input
type="range"
id="rampDown"
min="0"
max="120"
value="10">

</div>


<div class="card schedule">

<div>
START:
<span id="scheduleStart">--:--</span>
</div>

<div>
FULL BRIGHTNESS:
<span id="scheduleFull">--:--</span>
</div>

<div>
OFF:
<span id="scheduleOff">--:--</span>
</div>

</div>


<div class="buttons">

<button
onclick="manualOn()">
LIGHT ON
</button>

<button
class="off"
onclick="manualOff()">
LIGHT OFF
</button>

<button
class="save"
onclick="save()">
SAVE SETTINGS
</button>

</div>


<div
class="saveStatus"
id="saveStatus">

SETTINGS SAVED

</div>


<div class="footer">
GXDE LIGHT CONTROLLER
</div>

</div>


<script>

const $ =
id =>
document.getElementById(id);


// ==========================================================
// FORMAT TIME
// ==========================================================

function formatTime(
  totalMinutes
){

  totalMinutes =
    (
      totalMinutes % 1440 +
      1440
    ) % 1440;


  let h =
    Math.floor(
      totalMinutes / 60
    );


  let m =
    totalMinutes % 60;


  return (
    String(h).padStart(2,"0") +
    ":" +
    String(m).padStart(2,"0")
  );
}


// ==========================================================
// UPDATE SLIDER VALUES
// ==========================================================

function updateValues(){

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
    h + "h " +
    String(m).padStart(2,"0") +
    "m";


  $("brightnessValue").innerText =
    $("brightness").value +
    "%";


  $("rampUpValue").innerText =
    $("rampUp").value +
    " min";


  $("rampDownValue").innerText =
    $("rampDown").value +
    " min";


  updateSchedule();
}


// ==========================================================
// UPDATE SCHEDULE
// ==========================================================

function updateSchedule(){

  let startValue =
    $("start").value;


  if(!startValue){

    $("scheduleStart").innerText =
      "--:--";

    $("scheduleFull").innerText =
      "--:--";

    $("scheduleOff").innerText =
      "--:--";

    return;
  }


  let parts =
    startValue.split(":");


  let start =
    parseInt(parts[0]) * 60 +
    parseInt(parts[1]);


  let duration =
    parseInt(
      $("duration").value
    );


  let rampUp =
    parseInt(
      $("rampUp").value
    );


  $("scheduleStart").innerText =
    formatTime(start);


  $("scheduleFull").innerText =
    formatTime(
      start + rampUp
    );


  $("scheduleOff").innerText =
    formatTime(
      start + duration
    );
}


// ==========================================================
// LOAD ACTUAL SAVED SETTINGS
// ==========================================================

async function loadSettings(){

  try{

    let response =
      await fetch(
        "/settings"
      );


    if(!response.ok)
      return;


    let data =
      await response.json();


    $("start").value =
      String(
        data.startHour
      ).padStart(2,"0")
      +
      ":"
      +
      String(
        data.startMinute
      ).padStart(2,"0");


    $("duration").value =
      data.duration;


    $("brightness").value =
      data.brightness;


    $("rampUp").value =
      data.rampUp;


    $("rampDown").value =
      data.rampDown;


    updateValues();

    updateSchedule();


  }catch(e){

    console.log(
      "Failed to load settings",
      e
    );
  }
}


// ==========================================================
// CLOCK / STATUS
// ==========================================================

async function update(){

  try{

    let r =
      await fetch(
        "/status"
      );


    let d =
      await r.json();


    $("clock").innerText =
      d.time;


    $("status").innerText =
      "LIGHT " +
      (
        d.brightness > 0
          ? "ON"
          : "OFF"
      );


  }catch(e){

    $("clock").innerText =
      "--:--:--";
  }
}


// ==========================================================
// SAVE SETTINGS
// ==========================================================

async function save(){

  let params =
    new URLSearchParams({

      start:
        $("start").value,

      duration:
        $("duration").value,

      brightness:
        $("brightness").value,

      rampUp:
        $("rampUp").value,

      rampDown:
        $("rampDown").value
    });


  try{

    let response =
      await fetch(
        "/save?" +
        params.toString()
      );


    if(!response.ok){

      alert(
        "Failed to save settings"
      );

      return;
    }


    let data =
      await response.json();


    if(data.success){

      await loadSettings();


      let status =
        $("saveStatus");


      status.innerText =
        "SETTINGS SAVED";


      status.classList.add(
        "show"
      );


      setTimeout(
        () => {

          status.classList.remove(
            "show"
          );

        },
        2500
      );


    }else{

      alert(
        "Failed to save settings"
      );
    }


  }catch(e){

    alert(
      "Connection error while saving"
    );
  }
}


// ==========================================================
// MANUAL CONTROL
// ==========================================================

function manualOn(){

  fetch(
    "/manual?state=1"
  );
}


function manualOff(){

  fetch(
    "/manual?state=0"
  );
}


// ==========================================================
// SLIDERS
// ==========================================================

[
  "duration",
  "brightness",
  "rampUp",
  "rampDown"
].forEach(

  id =>

  $(id).addEventListener(
    "input",
    updateValues
  )
);


// ==========================================================
// START TIME
// ==========================================================

$("start").addEventListener(
  "change",
  updateSchedule
);


// ==========================================================
// INIT
// ==========================================================

async function init(){

  await loadSettings();

  update();
}


init();


setInterval(
  update,
  1000
);

</script>

</body>

</html>

)rawliteral";


// ============================================================
// ROOT
// ============================================================

void handleRoot() {

  if (
    apMode
  ) {

    server.send_P(
      200,
      "text/html",
      WIFI_HTML
    );

    return;
  }


  server.send_P(
    200,
    "text/html",
    INDEX_HTML
  );
}


// ============================================================
// STATUS
// ============================================================

void handleStatus() {

  int brightness =
    manualMode
      ? (
          manualState
            ? settings.maxBrightness
            : 0
        )
      : calculateBrightness();


  String json =
    "{";


  json +=
    "\"time\":\"";

  json +=
    getTimeString();

  json +=
    "\",";


  json +=
    "\"brightness\":";

  json +=
    brightness;


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// SETTINGS API
// ============================================================

void handleSettings() {

  String json =
    "{";


  json +=
    "\"startHour\":";

  json +=
    settings.startHour;

  json += ",";


  json +=
    "\"startMinute\":";

  json +=
    settings.startMinute;

  json += ",";


  json +=
    "\"duration\":";

  json +=
    settings.durationMinutes;

  json += ",";


  json +=
    "\"brightness\":";

  json +=
    settings.maxBrightness;

  json += ",";


  json +=
    "\"rampUp\":";

  json +=
    settings.rampUpMinutes;

  json += ",";


  json +=
    "\"rampDown\":";

  json +=
    settings.rampDownMinutes;


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// SAVE LIGHT SETTINGS
// ============================================================

void handleSave() {

  // ----------------------------------------------------------
  // START TIME
  // ----------------------------------------------------------

  if (
    server.hasArg("start")
  ) {

    String t =
      server.arg(
        "start"
      );


    if (
      t.length() >= 5
    ) {

      settings.startHour =
        t.substring(
          0,
          2
        ).toInt();


      settings.startMinute =
        t.substring(
          3,
          5
        ).toInt();
    }
  }


  // ----------------------------------------------------------
  // DURATION
  // ----------------------------------------------------------

  if (
    server.hasArg(
      "duration"
    )
  ) {

    settings.durationMinutes =
      server.arg(
        "duration"
      ).toInt();
  }


  // ----------------------------------------------------------
  // BRIGHTNESS
  // ----------------------------------------------------------

  if (
    server.hasArg(
      "brightness"
    )
  ) {

    settings.maxBrightness =
      server.arg(
        "brightness"
      ).toInt();
  }


  // ----------------------------------------------------------
  // RAMP UP
  // ----------------------------------------------------------

  if (
    server.hasArg(
      "rampUp"
    )
  ) {

    settings.rampUpMinutes =
      server.arg(
        "rampUp"
      ).toInt();
  }


  // ----------------------------------------------------------
  // RAMP DOWN
  // ----------------------------------------------------------

  if (
    server.hasArg(
      "rampDown"
    )
  ) {

    settings.rampDownMinutes =
      server.arg(
        "rampDown"
      ).toInt();
  }


  // ----------------------------------------------------------
  // VALIDATE
  // ----------------------------------------------------------

  settings.startHour =
    constrain(
      settings.startHour,
      0,
      23
    );


  settings.startMinute =
    constrain(
      settings.startMinute,
      0,
      59
    );


  settings.durationMinutes =
    constrain(
      settings.durationMinutes,
      1,
      1440
    );


  settings.maxBrightness =
    constrain(
      settings.maxBrightness,
      1,
      100
    );


  settings.rampUpMinutes =
    constrain(
      settings.rampUpMinutes,
      0,
      120
    );


  settings.rampDownMinutes =
    constrain(
      settings.rampDownMinutes,
      0,
      120
    );


  // ----------------------------------------------------------
  // SAVE
  // ----------------------------------------------------------

  bool success =
    saveSettings();


  manualMode =
    false;


  updateLight();


  // ----------------------------------------------------------
  // RESPONSE
  // ----------------------------------------------------------

  String json =
    "{";


  json +=
    "\"success\":";


  json +=
    success
      ? "true"
      : "false";


  json +=
    "}";


  server.send(
    success
      ? 200
      : 500,

    "application/json",

    json
  );
}


// ============================================================
// MANUAL CONTROL
// ============================================================

void handleManual() {

  if (
    !server.hasArg(
      "state"
    )
  ) {

    server.send(
      400,
      "text/plain",
      "Missing state"
    );

    return;
  }


  manualMode =
    true;


  manualState =
    server.arg(
      "state"
    ) == "1";


  updateLight();


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


// ============================================================
// WIFI SCAN
// ============================================================

void handleWiFiScan() {

  int count =
    WiFi.scanNetworks();


  String json =
    "[";


  for (
    int i = 0;
    i < count;
    i++
  ) {

    if (
      i > 0
    )
      json += ",";


    String name =
      WiFi.SSID(i);


    // --------------------------------------------------------
    // Basic JSON escaping
    // --------------------------------------------------------

    name.replace(
      "\\",
      "\\\\"
    );

    name.replace(
      "\"",
      "\\\""
    );


    json +=
      "{\"ssid\":\"";

    json +=
      name;

    json +=
      "\",\"rssi\":";

    json +=
      WiFi.RSSI(i);

    json +=
      "}";
  }


  json +=
    "]";


  WiFi.scanDelete();


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// SAVE WIFI CREDENTIALS
// ============================================================

void handleWiFiSave() {

  String ssid = "";
  String password = "";


  // ----------------------------------------------------------
  // Manual SSID has priority.
  // ----------------------------------------------------------

  if (
    server.hasArg(
      "manualssid"
    )
  ) {

    ssid =
      server.arg(
        "manualssid"
      );

    ssid.trim();
  }


  // ----------------------------------------------------------
  // If manual SSID is empty, use selected SSID.
  // ----------------------------------------------------------

  if (
    ssid.length() == 0 &&
    server.hasArg("ssid")
  ) {

    ssid =
      server.arg(
        "ssid"
      );

    ssid.trim();
  }


  if (
    server.hasArg(
      "password"
    )
  ) {

    password =
      server.arg(
        "password"
      );
  }


  // ----------------------------------------------------------
  // Validate
  // ----------------------------------------------------------

  if (
    ssid.length() == 0
  ) {

    server.send(
      400,
      "text/plain",
      "SSID is required"
    );

    return;
  }


  if (
    ssid.length() > 32
  ) {

    server.send(
      400,
      "text/plain",
      "SSID too long"
    );

    return;
  }


  if (
    password.length() > 64
  ) {

    server.send(
      400,
      "text/plain",
      "Password too long"
    );

    return;
  }


  // ----------------------------------------------------------
  // Save
  // ----------------------------------------------------------

  if (
    !saveWiFiCredentials(
      ssid,
      password
    )
  ) {

    server.send(
      500,
      "text/plain",
      "Failed to save WiFi credentials"
    );

    return;
  }


  // ----------------------------------------------------------
  // Send response before restarting.
  // ----------------------------------------------------------

  server.send(
    200,
    "text/html",

    "<html>"
    "<head>"
    "<meta name='viewport' "
    "content='width=device-width,initial-scale=1'>"
    "</head>"
    "<body style='background:#000;"
    "color:#00ffe1;font-family:Arial;"
    "text-align:center;padding-top:50px'>"
    "<h2>GXDE</h2>"
    "<p>Wi-Fi saved successfully.</p>"
    "<p>Restarting...</p>"
    "</body>"
    "</html>"
  );


  delay(1000);


  ESP.restart();
}


// ============================================================
// NTP
// ============================================================

void setupNTP() {

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {

    Serial.println(
      "NTP skipped - no internet connection"
    );

    return;
  }


  Serial.println(
    "Configuring NTP..."
  );


  // Philippines = UTC+8

  configTime(
    8 * 3600,
    0,
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
  );


  Serial.println(
    "NTP configured"
  );


  // ----------------------------------------------------------
  // Do NOT block for a long time.
  //
  // NTP will synchronize in the background.
  // This makes startup much faster.
  // ----------------------------------------------------------
}


// ============================================================
// FLASH BUTTON RESET
// ============================================================

void resetWiFiFromButton() {

  if (
    wifiResetTriggered
  )
    return;


  wifiResetTriggered =
    true;


  Serial.println();
  Serial.println(
    "===================================="
  );


  Serial.println(
    "FLASH BUTTON WIFI RESET"
  );


  // ----------------------------------------------------------
  // Delete only Wi-Fi credentials.
  // ----------------------------------------------------------

  deleteWiFiCredentials();


  Serial.println(
    "WiFi credentials erased."
  );


  Serial.println(
    "Release FLASH button."
  );


  // ----------------------------------------------------------
  // Wait for release.
  //
  // This is important because GPIO0 is the ESP8266
  // boot strap pin.
  // ----------------------------------------------------------

  unsigned long releaseStart =
    millis();


  while (
    digitalRead(
      WIFI_BUTTON_PIN
    ) == LOW
  ) {

    if (
      millis() -
      releaseStart >
      15000
    ) {

      Serial.println(
        "WARNING: FLASH still held."
      );

      Serial.println(
        "Waiting..."
      );

      releaseStart =
        millis();
    }


    delay(20);

    yield();
  }


  Serial.println(
    "FLASH released."
  );


  Serial.println(
    "Restarting ESP8266..."
  );


  delay(500);


  ESP.restart();
}


// ============================================================
// CHECK FLASH BUTTON
// ============================================================

void checkWiFiButton() {

  int state =
    digitalRead(
      WIFI_BUTTON_PIN
    );


  // ----------------------------------------------------------
  // Don't arm while button is held during boot.
  // ----------------------------------------------------------

  if (
    !wifiButtonArmed
  ) {

    if (
      state == HIGH
    ) {

      wifiButtonArmed =
        true;

      wifiButtonPrevious =
        HIGH;
    }

    return;
  }


  // ----------------------------------------------------------
  // PRESS
  // ----------------------------------------------------------

  if (
    state == LOW &&
    wifiButtonPrevious == HIGH
  ) {

    delay(
      WIFI_BUTTON_DEBOUNCE_MS
    );


    if (
      digitalRead(
        WIFI_BUTTON_PIN
      ) == LOW
    ) {

      wifiButtonPressStart =
        millis();


      Serial.println(
        "FLASH pressed"
      );
    }
  }


  // ----------------------------------------------------------
  // HOLD
  // ----------------------------------------------------------

  if (
    state == LOW &&
    wifiButtonPressStart != 0
  ) {

    unsigned long held =
      millis() -
      wifiButtonPressStart;


    if (
      held >=
      WIFI_BUTTON_HOLD_MS
    ) {

      resetWiFiFromButton();

      return;
    }
  }


  // ----------------------------------------------------------
  // RELEASE
  // ----------------------------------------------------------

  if (
    state == HIGH &&
    wifiButtonPrevious == LOW
  ) {

    wifiButtonPressStart =
      0;
  }


  wifiButtonPrevious =
    state;
}


// ============================================================
// SETUP WEB SERVER ROUTES
// ============================================================

void setupWebServer() {

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/status",
    handleStatus
  );


  server.on(
    "/settings",
    handleSettings
  );


  server.on(
    "/save",
    handleSave
  );


  server.on(
    "/manual",
    handleManual
  );


  // ----------------------------------------------------------
  // WiFi AP routes
  // ----------------------------------------------------------

  server.on(
    "/wifi/scan",
    HTTP_GET,
    handleWiFiScan
  );


  server.on(
    "/wifi/save",
    HTTP_POST,
    handleWiFiSave
  );


  server.begin();


  Serial.println(
    "Web server started"
  );
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(
    115200
  );


  delay(300);


  Serial.println();
  Serial.println(
    "===================================="
  );

  Serial.println(
    "GXDE LIGHT CONTROLLER"
  );

  Serial.println(
    "===================================="
  );


  // ==========================================================
  // FLASH BUTTON
  // ==========================================================

  pinMode(
    WIFI_BUTTON_PIN,
    INPUT_PULLUP
  );


  wifiButtonPrevious =
    digitalRead(
      WIFI_BUTTON_PIN
    );


  if (
    wifiButtonPrevious == HIGH
  ) {

    wifiButtonArmed =
      true;
  }


  // ==========================================================
  // PWM
  // ==========================================================

  pinMode(
    PWM_PIN,
    OUTPUT
  );


  analogWriteRange(
    PWM_MAX
  );


  setBrightness(
    0
  );


  // ==========================================================
  // LITTLEFS
  // ==========================================================

  if (
    !LittleFS.begin()
  ) {

    Serial.println(
      "ERROR: LittleFS failed"
    );

    return;
  }


  Serial.println(
    "LittleFS mounted"
  );


  // ==========================================================
  // LOAD LIGHT SETTINGS
  // ==========================================================

  loadSettings();


  // ==========================================================
  // LOAD WIFI CREDENTIALS
  // ==========================================================

  bool hasWiFi =
    loadWiFiCredentials();


  // ==========================================================
  // WIFI
  // ==========================================================

  bool connected =
    false;


  if (
    hasWiFi
  ) {

    connected =
      connectToWiFi();
  }


  // ----------------------------------------------------------
  // If no credentials OR connection failed:
  //
  // Start AP immediately.
  // ----------------------------------------------------------

  if (
    !connected
  ) {

    startAccessPoint();

  } else {

    apMode =
      false;
  }


  // ==========================================================
  // NTP
  // ==========================================================

  setupNTP();


  // ==========================================================
  // WEB SERVER
  // ==========================================================

  setupWebServer();


  // ==========================================================
  // LIGHT SAFETY
  // ==========================================================

  manualMode =
    false;

  manualState =
    false;


  setBrightness(
    0
  );


  // ==========================================================
  // READY
  // ==========================================================

  Serial.println();
  Serial.println(
    "===================================="
  );


  Serial.println(
    "GXDE LIGHT READY"
  );


  Serial.print(
    "Schedule: "
  );


  Serial.print(
    getScheduleStart()
  );


  Serial.print(
    " -> "
  );


  Serial.println(
    getScheduleOff()
  );


  Serial.print(
    "Full brightness: "
  );


  Serial.println(
    getScheduleFullBrightness()
  );


  if (
    apMode
  ) {

    Serial.println();
    Serial.println(
      "WIFI SETUP MODE"
    );


    Serial.print(
      "Connect to: "
    );


    Serial.println(
      AP_NAME
    );


    Serial.print(
      "Password: "
    );


    Serial.println(
      AP_PASSWORD
    );


    Serial.print(
      "Open: http://"
    );


    Serial.println(
      WiFi.softAPIP()
    );

  } else {

    Serial.println();
    Serial.print(
      "GXDE IP: "
    );


    Serial.println(
      WiFi.localIP()
    );
  }


  Serial.println();
  Serial.println(
    "FLASH hold 5 seconds = reset WiFi"
  );


  Serial.println(
    "===================================="
  );
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // Web server
  // ----------------------------------------------------------

  server.handleClient();


  // ----------------------------------------------------------
  // FLASH button
  // ----------------------------------------------------------

  checkWiFiButton();


  // ----------------------------------------------------------
  // Update light once per second
  // ----------------------------------------------------------

  if (
    millis() -
    lastUpdate >=
    1000
  ) {

    lastUpdate =
      millis();


    updateLight();
  }


  yield();
}
