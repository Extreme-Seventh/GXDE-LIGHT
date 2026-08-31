/*
   ============================================================
   GXDE SMART AQUARIUM LIGHT CONTROLLER
   ESP8266 / NodeMCU V3
   ============================================================

   HARDWARE
   ------------------------------------------------------------
   ESP8266
   DS1302 RTC
   Single-channel MOSFET
   5V USB grow light

   FEATURES
   ------------------------------------------------------------
   - DS1302 RTC primary time source
   - NTP synchronization
   - Philippines UTC+8
   - Correct UTC -> local time conversion
   - RTC updated from NTP
   - Automatic recovery after power interruption
   - Smooth sunrise
   - Smooth sunset
   - Adjustable maximum brightness
   - Adjustable duration
   - Plant acclimation
   - Presets
   - Manual brightness
   - Temporary boost
   - Optional midday / siesta
   - 24-hour graphical schedule
   - Wi-Fi auto reconnect
   - Automatic AP fallback
   - Wi-Fi credentials saved separately
   - Lighting settings saved to LittleFS
   - Event log
   - System diagnostics
   - Mobile responsive Web UI
   - PWM output on D5
   - No RGB/color control
   ============================================================


   ============================================================
   IMPORTANT LIBRARY
   ============================================================

   Install:

       ErriezDS1302

   The library supports:

       rtc.begin()
       rtc.read(&tm)
       rtc.write(&tm)
       rtc.isRunning()
       rtc.clockEnable()

   ============================================================


   ============================================================
   PINOUT
   ============================================================

   PWM:
       D5

   DS1302:
       D1 = CLK
       D2 = IO
       D6 = CE

   FLASH:
       GPIO0 / D3


   ============================================================
*/

#include "wifi_ui.h"
#include "main_ui.h"
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <LittleFS.h>

#include <time.h>

#include <ErriezDS1302.h>


// ============================================================
// HARDWARE
// ============================================================

#define PWM_PIN D5

#define DS1302_CLK_PIN D1
#define DS1302_IO_PIN D2
#define DS1302_CE_PIN D6

#define WIFI_BUTTON_PIN 0

#define PWM_MAX 1023


// ============================================================
// DS1302
// ============================================================

ErriezDS1302 rtc(
  DS1302_CLK_PIN,
  DS1302_IO_PIN,
  DS1302_CE_PIN);


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
// TIME
// ============================================================

const char* TIME_ZONE =
  "PHT-8";

bool rtcAvailable = false;

bool ntpSynced = false;

unsigned long lastNtpSync = 0;

const unsigned long NTP_RESYNC_INTERVAL =
  6UL * 60UL * 60UL * 1000UL;


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

  bool siestaEnabled;

  int siestaStartHour;
  int siestaStartMinute;

  int siestaEndHour;
  int siestaEndMinute;

  bool acclimationEnabled;

  int acclimationStartBrightness;
  int acclimationTargetBrightness;

  int acclimationDays;

  int acclimationDay;
  uint8_t preset;
};

Settings settings;


// ============================================================
// WIFI CREDENTIALS
// ============================================================

String wifiSSID = "";

String wifiPassword = "";

bool apMode = false;


// ============================================================
// LIGHT RUNTIME
// ============================================================

bool manualMode = false;

int manualBrightness = 0;

bool boostMode = false;

int boostBrightness = 100;

unsigned long boostEndMillis = 0;

int currentBrightness = 0;

unsigned long lastLightUpdate = 0;


// ============================================================
// WIFI RUNTIME
// ============================================================

unsigned long lastWiFiRetry = 0;

unsigned long wifiDisconnectedSince = 0;

const unsigned long WIFI_RETRY_INTERVAL =
  30000UL;

const unsigned long WIFI_FAIL_TO_AP_TIME =
  120000UL;


// ============================================================
// NTP
// ============================================================

unsigned long ntpStartMillis = 0;

bool ntpWaiting = false;


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
// EVENT LOG
// ============================================================

#define EVENT_COUNT 30

String eventLog[EVENT_COUNT];

int eventLogIndex = 0;


// ============================================================
// DEFAULT SETTINGS
// ============================================================

void defaultSettings() {
  settings.preset = 0;

  settings.startHour =
    18;

  settings.startMinute =
    0;

  settings.durationMinutes =
    720;

  settings.rampUpMinutes =
    30;

  settings.rampDownMinutes =
    30;

  settings.maxBrightness =
    100;


  settings.siestaEnabled =
    false;

  settings.siestaStartHour =
    12;

  settings.siestaStartMinute =
    0;

  settings.siestaEndHour =
    14;

  settings.siestaEndMinute =
    0;


  settings.acclimationEnabled =
    false;

  settings.acclimationStartBrightness =
    40;

  settings.acclimationTargetBrightness =
    100;

  settings.acclimationDays =
    14;

  settings.acclimationDay =
    1;

  settings.preset =
    0;
}


// ============================================================
// EVENT LOG
// ============================================================

void addEvent(
  String message) {

  String timestamp =
    getTimeString();

  eventLog[eventLogIndex] =
    timestamp + "  " + message;

  eventLogIndex++;

  if (
    eventLogIndex >= EVENT_COUNT)
    eventLogIndex = 0;

  Serial.print(
    "[EVENT] ");

  Serial.println(
    eventLog[eventLogIndex == 0
               ? EVENT_COUNT - 1
               : eventLogIndex - 1]);
}


// ============================================================
// SAVE SETTINGS
// ============================================================

bool saveSettings() {

  File file =
    LittleFS.open(
      "/settings.txt",
      "w");

  if (!file) {

    Serial.println(
      "ERROR: Cannot save settings");

    return false;
  }


  file.println(
    settings.startHour);

  file.println(
    settings.startMinute);

  file.println(
    settings.durationMinutes);

  file.println(
    settings.rampUpMinutes);

  file.println(
    settings.rampDownMinutes);

  file.println(
    settings.maxBrightness);

  file.println(
    settings.siestaEnabled ? 1 : 0);

  file.println(
    settings.siestaStartHour);

  file.println(
    settings.siestaStartMinute);

  file.println(
    settings.siestaEndHour);

  file.println(
    settings.siestaEndMinute);

  file.println(
    settings.acclimationEnabled ? 1 : 0);

  file.println(
    settings.acclimationStartBrightness);

  file.println(
    settings.acclimationTargetBrightness);

  file.println(
    settings.acclimationDays);

  file.println(
    settings.acclimationDay);

  file.println(
    settings.preset);


  file.close();

  return true;
}


// ============================================================
// LOAD SETTINGS
// ============================================================

bool loadSettings() {

  if (
    !LittleFS.exists(
      "/settings.txt")) {

    defaultSettings();

    saveSettings();

    return false;
  }


  File file =
    LittleFS.open(
      "/settings.txt",
      "r");

  if (!file) {

    defaultSettings();

    return false;
  }


  settings.startHour =
    file.readStringUntil('\n').toInt();

  settings.startMinute =
    file.readStringUntil('\n').toInt();

  settings.durationMinutes =
    file.readStringUntil('\n').toInt();

  settings.rampUpMinutes =
    file.readStringUntil('\n').toInt();

  settings.rampDownMinutes =
    file.readStringUntil('\n').toInt();

  settings.maxBrightness =
    file.readStringUntil('\n').toInt();

  settings.siestaEnabled =
    file.readStringUntil('\n').toInt() != 0;

  settings.siestaStartHour =
    file.readStringUntil('\n').toInt();

  settings.siestaStartMinute =
    file.readStringUntil('\n').toInt();

  settings.siestaEndHour =
    file.readStringUntil('\n').toInt();

  settings.siestaEndMinute =
    file.readStringUntil('\n').toInt();

  settings.acclimationEnabled =
    file.readStringUntil('\n').toInt() != 0;

  settings.acclimationStartBrightness =
    file.readStringUntil('\n').toInt();

  settings.acclimationTargetBrightness =
    file.readStringUntil('\n').toInt();

  settings.acclimationDays =
    file.readStringUntil('\n').toInt();

  settings.acclimationDay =
    file.readStringUntil('\n').toInt();

  if (file.available()) {

    settings.preset =
      file.readStringUntil('\n').toInt();

  } else {

    settings.preset =
      0;
  }

  file.close();


  settings.startHour =
    constrain(
      settings.startHour,
      0,
      23);

  settings.startMinute =
    constrain(
      settings.startMinute,
      0,
      59);

  settings.durationMinutes =
    constrain(
      settings.durationMinutes,
      1,
      1440);

  settings.rampUpMinutes =
    constrain(
      settings.rampUpMinutes,
      0,
      120);

  settings.rampDownMinutes =
    constrain(
      settings.rampDownMinutes,
      0,
      120);

  settings.maxBrightness =
    constrain(
      settings.maxBrightness,
      1,
      100);

  settings.siestaStartHour =
    constrain(
      settings.siestaStartHour,
      0,
      23);

  settings.siestaStartMinute =
    constrain(
      settings.siestaStartMinute,
      0,
      59);

  settings.siestaEndHour =
    constrain(
      settings.siestaEndHour,
      0,
      23);

  settings.siestaEndMinute =
    constrain(
      settings.siestaEndMinute,
      0,
      59);

  settings.acclimationStartBrightness =
    constrain(
      settings.acclimationStartBrightness,
      1,
      100);

  settings.acclimationTargetBrightness =
    constrain(
      settings.acclimationTargetBrightness,
      1,
      100);

  settings.acclimationDays =
    constrain(
      settings.acclimationDays,
      1,
      365);

  settings.acclimationDay =
    constrain(
      settings.acclimationDay,
      1,
      settings.acclimationDays);


  return true;
}


// ============================================================
// WIFI SAVE
// ============================================================

bool saveWiFiCredentials(
  String ssid,
  String password) {

  File file =
    LittleFS.open(
      "/wifi.txt",
      "w");

  if (!file)
    return false;


  file.println(
    ssid);

  file.println(
    password);

  file.close();


  wifiSSID =
    ssid;

  wifiPassword =
    password;


  return true;
}


// ============================================================
// WIFI LOAD
// ============================================================

bool loadWiFiCredentials() {

  if (
    !LittleFS.exists(
      "/wifi.txt")) {

    wifiSSID = "";
    wifiPassword = "";

    return false;
  }


  File file =
    LittleFS.open(
      "/wifi.txt",
      "r");

  if (!file)
    return false;


  wifiSSID =
    file.readStringUntil('\n');

  wifiPassword =
    file.readStringUntil('\n');


  file.close();


  wifiSSID.trim();
  wifiPassword.trim();


  return wifiSSID.length() > 0;
}


// ============================================================
// WIFI DELETE
// ============================================================

void deleteWiFiCredentials() {

  if (
    LittleFS.exists(
      "/wifi.txt")) {

    LittleFS.remove(
      "/wifi.txt");
  }


  wifiSSID = "";

  wifiPassword = "";


  addEvent(
    "WiFi credentials erased");
}


// ============================================================
// PWM
// ============================================================

void setBrightness(
  int percent) {

  percent =
    constrain(
      percent,
      0,
      100);


  currentBrightness =
    percent;


  int pwm =
    map(
      percent,
      0,
      100,
      0,
      PWM_MAX);


  analogWrite(
    PWM_PIN,
    pwm);
}


// ============================================================
// TIMEZONE
// ============================================================

void setupTimezone() {

  /*
     PHT-8 means:

     Local time = UTC + 8 hours

     There is no daylight saving time
     in the Philippines.
  */

  setenv(
    "TZ",
    TIME_ZONE,
    1);

  tzset();
}


// ============================================================
// SYSTEM TIME VALID
// ============================================================

bool isSystemTimeValid() {

  time_t now =
    time(nullptr);

  return now > 1700000000;
}


// ============================================================
// FORMAT SYSTEM TIME
// ============================================================

String getTimeString() {

  if (
    !isSystemTimeValid())
    return "--:--:--";


  time_t now =
    time(nullptr);


  struct tm localTime;


  localtime_r(
    &now,
    &localTime);


  char buffer[20];


  snprintf(
    buffer,
    sizeof(buffer),
    "%02d:%02d:%02d",
    localTime.tm_hour,
    localTime.tm_min,
    localTime.tm_sec);


  return String(
    buffer);
}


// ============================================================
// GET LOCAL STRUCT TM
// ============================================================

bool getLocalTimeStruct(
  struct tm* result) {

  if (
    !isSystemTimeValid())
    return false;


  time_t now =
    time(nullptr);


  localtime_r(
    &now,
    result);


  return true;
}


// ============================================================
// MINUTES SINCE MIDNIGHT
// ============================================================

int minutesSinceMidnight() {

  struct tm t;


  if (
    !getLocalTimeStruct(
      &t))
    return -1;


  return t.tm_hour * 60 + t.tm_min;
}


// ============================================================
// READ RTC
// ============================================================

bool readRTC(
  struct tm* result) {

  if (
    !rtcAvailable)
    return false;


  if (
    !rtc.read(
      result))
    return false;


  return true;
}


// ============================================================
// SET RTC FROM LOCAL SYSTEM TIME
// ============================================================

bool updateRTCFromSystemTime() {

  if (
    !rtcAvailable)
    return false;


  struct tm localTime;


  if (
    !getLocalTimeStruct(
      &localTime))
    return false;


  /*
     IMPORTANT:

     The DS1302 is being treated as a
     LOCAL Philippines clock.

     We therefore write:

         hour
         minute
         second
         date
         month
         year

     from localTime.

     We do NOT add +8 hours here.
  */


  if (
    !rtc.write(
      &localTime))
    return false;


  Serial.println(
    "DS1302 updated from NTP/system local time.");


  return true;
}


// ============================================================
// LOAD RTC INTO SYSTEM CLOCK
// ============================================================

bool loadSystemTimeFromRTC() {

  if (
    !rtcAvailable)
    return false;


  struct tm rtcTime;


  if (
    !rtc.read(
      &rtcTime))
    return false;


  /*
     The DS1302 stores Philippine local
     calendar/time.

     Convert that local time to a Unix
     epoch using the Philippines timezone.
  */


  time_t localEpoch =
    mktime(
      &rtcTime);


  if (
    localEpoch <= 0)
    return false;


  /*
     The ESP8266 system clock uses epoch time.

     We set the system clock to the epoch
     representing the same Philippine local
     clock reading.
  */


  timeval tv;

  tv.tv_sec =
    localEpoch;

  tv.tv_usec =
    0;


  settimeofday(
    &tv,
    nullptr);


  Serial.println(
    "System time restored from DS1302.");


  Serial.print(
    "RTC local time: ");

  Serial.println(
    getTimeString());


  return true;
}


// ============================================================
// INITIALIZE RTC
// ============================================================

void setupRTC() {

  Serial.println();
  Serial.println(
    "Initializing DS1302...");


  rtcAvailable =
    rtc.begin();


  if (
    !rtcAvailable) {

    Serial.println(
      "ERROR: DS1302 not detected");

    return;
  }


  if (
    !rtc.isRunning()) {

    Serial.println(
      "RTC oscillator is stopped.");


    rtc.clockEnable(
      true);


    Serial.println(
      "RTC oscillator enabled.");

  } else {

    Serial.println(
      "DS1302 oscillator running.");
  }


  /*
     Attempt to recover time immediately.

     If RTC contains a valid date/time,
     use it before Wi-Fi/NTP is available.
  */


  struct tm rtcTime;


  if (
    rtc.read(
      &rtcTime)) {

    int year =
      rtcTime.tm_year + 1900;


    if (
      year >= 2024 && year <= 2099) {

      loadSystemTimeFromRTC();

      addEvent(
        "Time restored from DS1302");

    } else {

      Serial.println(
        "RTC date appears invalid.");
    }

  } else {

    Serial.println(
      "RTC read failed.");
  }
}


// ============================================================
// NTP START
// ============================================================

void startNTP() {

  if (
    WiFi.status() != WL_CONNECTED)
    return;


  /*
     IMPORTANT:

     We configure the ESP system clock
     using UTC.

     The TZ environment converts UTC
     to Philippines local time whenever
     localtime() / localtime_r() is used.

     This prevents the previous +8 hour
     error.
  */


  configTime(
    8 * 3600,
    0,
    "ph.pool.ntp.org",
    "time.nist.gov",
    "time.google.com");


  ntpStartMillis =
    millis();

  ntpWaiting =
    true;


  Serial.println();
  Serial.println(
    "Waiting for NTP synchronization...");
}


// ============================================================
// CHECK NTP
// ============================================================

void checkNTP() {

  if (
    WiFi.status() != WL_CONNECTED)
    return;


  if (
    !ntpWaiting)
    return;


  if (
    isSystemTimeValid()) {

    ntpWaiting =
      false;

    ntpSynced =
      true;

    lastNtpSync =
      millis();


    Serial.println();
    Serial.println(
      "====================================");

    Serial.println(
      "NTP TIME SYNCHRONIZED");

    Serial.print(
      "Correct Philippines time: ");

    Serial.println(
      getTimeString());


    /*
       NOW write the corrected LOCAL
       Philippines time to DS1302.
    */

    if (
      updateRTCFromSystemTime()) {

      Serial.println(
        "RTC synchronization complete.");

      addEvent(
        "NTP synchronized / RTC updated");

    } else {

      Serial.println(
        "WARNING: RTC update failed.");
    }


    Serial.println(
      "====================================");
  }


  /*
     If NTP has not synchronized after
     30 seconds, stop waiting.

     The RTC continues operating.
  */

  if (
    millis() - ntpStartMillis > 30000UL) {

    ntpWaiting =
      false;


    Serial.println(
      "NTP timeout.");

    addEvent(
      "NTP synchronization timeout");
  }
}


// ============================================================
// PERIODIC NTP
// ============================================================

void periodicNTP() {

  if (
    WiFi.status() != WL_CONNECTED)
    return;


  if (
    !ntpSynced)
    return;


  if (
    millis() - lastNtpSync >= NTP_RESYNC_INTERVAL) {

    ntpSynced =
      false;


    startNTP();
  }
}


// ============================================================
// WIFI CONNECT
// ============================================================

bool connectToWiFi() {

  if (
    wifiSSID.length() == 0)
    return false;


  Serial.println();
  Serial.println(
    "Connecting to saved WiFi...");

  Serial.print(
    "SSID: ");

  Serial.println(
    wifiSSID);


  apMode =
    false;


  WiFi.mode(
    WIFI_STA);


  WiFi.begin(
    wifiSSID.c_str(),
    wifiPassword.c_str());


  unsigned long start =
    millis();


  while (
    WiFi.status() != WL_CONNECTED && millis() - start < 10000UL) {

    delay(100);

    yield();
  }


  if (
    WiFi.status() == WL_CONNECTED) {

    Serial.println();

    Serial.println(
      "WiFi connected.");

    Serial.print(
      "IP: ");

    Serial.println(
      WiFi.localIP());


    Serial.print(
      "RSSI: ");

    Serial.println(
      WiFi.RSSI());


    wifiDisconnectedSince =
      0;


    addEvent(
      "WiFi connected");


    return true;
  }


  Serial.println(
    "WiFi connection failed.");


  return false;
}


// ============================================================
// START AP
// ============================================================

void startAccessPoint() {

  apMode =
    true;


  WiFi.disconnect();

  delay(100);


  WiFi.mode(
    WIFI_AP);


  bool result =
    WiFi.softAP(
      AP_NAME,
      AP_PASSWORD);


  Serial.println();
  Serial.println(
    "====================================");

  Serial.println(
    "GXDE AP MODE");


  if (
    result) {

    Serial.print(
      "SSID: ");

    Serial.println(
      AP_NAME);

    Serial.print(
      "Password: ");

    Serial.println(
      AP_PASSWORD);

    Serial.print(
      "IP: ");

    Serial.println(
      WiFi.softAPIP());


    addEvent(
      "AP mode started");

  } else {

    Serial.println(
      "ERROR: AP failed");
  }


  Serial.println(
    "====================================");
}


// ============================================================
// WIFI MONITOR
// ============================================================

void updateWiFi() {

  if (
    apMode)
    return;


  if (
    WiFi.status() == WL_CONNECTED) {

    wifiDisconnectedSince =
      0;

    return;
  }


  if (
    wifiDisconnectedSince == 0) {

    wifiDisconnectedSince =
      millis();


    Serial.println(
      "WiFi disconnected.");

    addEvent(
      "WiFi disconnected");
  }


  if (
    millis() - lastWiFiRetry >= WIFI_RETRY_INTERVAL) {

    lastWiFiRetry =
      millis();


    Serial.println(
      "Retrying WiFi...");


    WiFi.disconnect();

    WiFi.begin(
      wifiSSID.c_str(),
      wifiPassword.c_str());
  }


  /*
     If WiFi remains unavailable,
     switch to AP mode.

     IMPORTANT:

     The light keeps running from
     DS1302/system time.
  */

  if (
    millis() - wifiDisconnectedSince >= WIFI_FAIL_TO_AP_TIME) {

    Serial.println(
      "WiFi unavailable.");

    Serial.println(
      "Switching to GXDE AP.");


    startAccessPoint();
  }
}


// ============================================================
// MINUTES WRAP
// ============================================================

int normalizeMinutes(
  int value) {

  value %= 1440;

  if (
    value < 0)
    value += 1440;

  return value;
}


// ============================================================
// IS BETWEEN TIME
// ============================================================

bool isBetweenMinutes(
  int now,
  int start,
  int end) {

  start =
    normalizeMinutes(start);

  end =
    normalizeMinutes(end);

  now =
    normalizeMinutes(now);


  if (
    start == end)
    return false;


  if (
    start < end) {

    return now >= start && now < end;
  }


  return now >= start || now < end;
}


// ============================================================
// EFFECTIVE MAX BRIGHTNESS
// ============================================================

int getEffectiveMaximumBrightness() {

  int target =
    settings.maxBrightness;


  if (
    !settings.acclimationEnabled)
    return target;


  if (
    settings.acclimationDays <= 1)
    return target;


  int start =
    settings.acclimationStartBrightness;


  int end =
    settings.acclimationTargetBrightness;


  int day =
    constrain(
      settings.acclimationDay,
      1,
      settings.acclimationDays);


  float progress =
    (float)(day - 1) / (float)(settings.acclimationDays - 1);


  int acclimated =
    start + (int)((end - start) * progress);


  /*
     Never exceed user's configured
     maximum brightness.
  */

  return constrain(
    acclimated,
    1,
    target);
}


// ============================================================
// SMOOTHSTEP
// ============================================================

float smoothStep(
  float x) {

  x =
    constrain(
      x,
      0.0f,
      1.0f);


  /*
     Smooth S curve:

       0 -> 0
       0.5 -> 0.5
       1 -> 1

     This produces a much smoother
     sunrise/sunset than simple linear
     stepping.
  */

  return x * x * (3.0f - 2.0f * x);
}


// ============================================================
// GET CURRENT LOCAL SECONDS
// ============================================================

long secondsSinceMidnight() {

  struct tm t;


  if (
    !getLocalTimeStruct(
      &t))
    return -1;


  return (long)t.tm_hour * 3600L + (long)t.tm_min * 60L + (long)t.tm_sec;
}


// ============================================================
// SCHEDULE BRIGHTNESS
// ============================================================

int calculateScheduleBrightness() {

  long nowSeconds =
    secondsSinceMidnight();


  if (
    nowSeconds < 0)
    return 0;


  long startSeconds =
    (settings.startHour * 3600L
     + settings.startMinute * 60L);


  long durationSeconds =
    settings.durationMinutes * 60L;


  long rampUpSeconds =
    settings.rampUpMinutes * 60L;


  long rampDownSeconds =
    settings.rampDownMinutes * 60L;


  /*
     Convert current time into elapsed
     seconds since schedule start.

     Handles midnight correctly.
  */

  long elapsed =
    nowSeconds - startSeconds;


  if (
    elapsed < 0)
    elapsed += 86400L;


  /*
     Duration = 1440 means continuous
     schedule.
  */

  if (
    settings.durationMinutes < 1440) {

    if (
      elapsed >= durationSeconds)
      return 0;
  }


  /*
     SIesta.

     During the configured break,
     light is off.
  */

  if (
    settings.siestaEnabled) {

    int nowMinute =
      (int)(nowSeconds / 60L);

    int siestaStart =
      settings.siestaStartHour * 60 + settings.siestaStartMinute;

    int siestaEnd =
      settings.siestaEndHour * 60 + settings.siestaEndMinute;


    if (
      isBetweenMinutes(
        nowMinute,
        siestaStart,
        siestaEnd)) {

      return 0;
    }
  }


  int maxBrightness =
    getEffectiveMaximumBrightness();


  /*
     RAMP UP
  */

  if (
    rampUpSeconds > 0 && elapsed < rampUpSeconds) {

    float progress =
      (float)elapsed / (float)rampUpSeconds;


    progress =
      smoothStep(
        progress);


    return constrain(
      (int)(progress * maxBrightness),
      0,
      maxBrightness);
  }


  /*
     RAMP DOWN
  */

  if (
    rampDownSeconds > 0 && settings.durationMinutes < 1440) {

    long rampDownStart =
      durationSeconds - rampDownSeconds;


    if (
      elapsed >= rampDownStart) {

      float progress =
        (float)(elapsed - rampDownStart)
        / (float)rampDownSeconds;


      progress =
        smoothStep(
          progress);


      float remaining =
        1.0f - progress;


      return constrain(
        (int)(remaining * maxBrightness),
        0,
        maxBrightness);
    }
  }


  /*
     FULL BRIGHTNESS
  */

  return maxBrightness;
}


// ============================================================
// UPDATE LIGHT
// ============================================================

void updateLight() {

  int target = 0;


  /*
     BOOST
  */

  if (
    boostMode) {

    if (
      millis() >= boostEndMillis) {

      boostMode =
        false;

      addEvent(
        "Temporary boost finished");

    } else {

      target =
        boostBrightness;
    }
  }


  /*
     MANUAL
  */

  if (
    !boostMode && manualMode) {

    target =
      manualBrightness;
  }


  /*
     AUTO
  */

  if (
    !boostMode && !manualMode) {

    target =
      calculateScheduleBrightness();
  }


  /*
     Immediate PWM update.

     Because this function runs every
     100 ms, brightness follows the
     schedule smoothly rather than
     changing only once per minute.
  */

  setBrightness(
    target);
}


// ============================================================
// SCHEDULE FORMAT
// ============================================================

String formatMinutes(
  int totalMinutes) {

  totalMinutes =
    normalizeMinutes(
      totalMinutes);


  int h =
    totalMinutes / 60;

  int m =
    totalMinutes % 60;


  char buffer[8];


  snprintf(
    buffer,
    sizeof(buffer),
    "%02d:%02d",
    h,
    m);


  return String(
    buffer);
}


// ============================================================
// SCHEDULE START
// ============================================================

int scheduleStartMinutes() {

  return settings.startHour * 60 + settings.startMinute;
}


// ============================================================
// FULL BRIGHTNESS TIME
// ============================================================

int scheduleFullBrightnessMinutes() {

  return scheduleStartMinutes() + settings.rampUpMinutes;
}


// ============================================================
// SCHEDULE OFF
// ============================================================

int scheduleOffMinutes() {

  return scheduleStartMinutes() + settings.durationMinutes;
}


// ============================================================
// NEXT EVENT
// ============================================================

String getNextEvent() {

  int now =
    minutesSinceMidnight();


  if (
    now < 0)
    return "Waiting for time";


  int start =
    scheduleStartMinutes();


  int full =
    scheduleFullBrightnessMinutes();


  int off =
    scheduleOffMinutes();


  if (
    isBetweenMinutes(
      now,
      start,
      full)) {

    return "Full brightness " + formatMinutes(full);
  }


  if (
    isBetweenMinutes(
      now,
      full,
      off)) {

    return "Sunset " + formatMinutes(off - settings.rampDownMinutes);
  }


  return "Sunrise " + formatMinutes(start);
}



// ============================================================
// ROOT
// ============================================================

void handleRoot() {

  if (
    apMode) {

    server.send_P(
      200,
      "text/html",
      WIFI_HTML);

    return;
  }


  server.send_P(
    200,
    "text/html",
    INDEX_HTML);
}


// ============================================================
// STATUS API
// ============================================================

void handleStatus() {

  int pwm =
    map(
      currentBrightness,
      0,
      100,
      0,
      PWM_MAX);


  String state;


  if (
    boostMode)
    state =
      "BOOST";

  else if (
    manualMode)
    state =
      "MANUAL";

  else {

    if (
      currentBrightness > 0)
      state =
        "AUTO - LIGHT ON";

    else
      state =
        "AUTO - LIGHT OFF";
  }


  String json =
    "{";


  json +=
    "\"time\":\"" + getTimeString() + "\",";


  json +=
    "\"brightness\":" + String(currentBrightness) + ",";


  json +=
    "\"state\":\"" + state + "\",";


  json +=
    "\"mode\":\"" + String(boostMode ? "BOOST" : manualMode ? "MANUAL"
                                                            : "AUTO")
    + "\",";


  json +=
    "\"rtc\":\"" + String(rtcAvailable ? "OK" : "ERROR") + "\",";


  json +=
    "\"ntp\":\"" + String(ntpSynced ? "SYNCHRONIZED" : "NOT SYNCED") + "\",";


  json +=
    "\"timeSource\":\"" + String(rtcAvailable ? "DS1302 / NTP" : "SYSTEM") + "\",";


  json +=
    "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : apMode ? "AP MODE"
                                                                                : "DISCONNECTED")
    + "\",";


  json +=
    "\"rssi\":" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0) + ",";


  json +=
    "\"pwm\":" + String(pwm) + ",";


  json +=
    "\"effectiveMax\":" + String(getEffectiveMaximumBrightness()) + ",";


  json +=
    "\"nextEvent\":\"" + getNextEvent() + "\"";


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json);
}


// ============================================================
// SETTINGS API
// ============================================================

void handleSettings() {

  String json =
    "{";


  json +=
    "\"startHour\":" + String(settings.startHour) + ",";


  json +=
    "\"startMinute\":" + String(settings.startMinute) + ",";


  json +=
    "\"duration\":" + String(settings.durationMinutes) + ",";


  json +=
    "\"brightness\":" + String(settings.maxBrightness) + ",";


  json +=
    "\"rampUp\":" + String(settings.rampUpMinutes) + ",";


  json +=
    "\"rampDown\":" + String(settings.rampDownMinutes) + ",";


  json +=
    "\"siestaEnabled\":" + String(settings.siestaEnabled ? "true" : "false") + ",";


  json +=
    "\"siestaStartHour\":" + String(settings.siestaStartHour) + ",";


  json +=
    "\"siestaStartMinute\":" + String(settings.siestaStartMinute) + ",";


  json +=
    "\"siestaEndHour\":" + String(settings.siestaEndHour) + ",";


  json +=
    "\"siestaEndMinute\":" + String(settings.siestaEndMinute) + ",";


  json +=
    "\"acclimationEnabled\":" + String(settings.acclimationEnabled ? "true" : "false") + ",";


  json +=
    "\"acclimationStart\":" + String(settings.acclimationStartBrightness) + ",";


  json +=
    "\"acclimationTarget\":" + String(settings.acclimationTargetBrightness) + ",";


  json +=
    "\"acclimationDays\":" + String(settings.acclimationDays) + ",";


  json +=
    "\"acclimationDay\":" + String(settings.acclimationDay) + ",";

json +=
  "\"preset\":\"" +
  String(
    settings.preset == 1 ? "LOW TECH" :
    settings.preset == 2 ? "PLANTED" :
    settings.preset == 3 ? "BRIGHT PLANTED" :
    "CUSTOM"
  ) +
  "\"";


  json +=
    "}";



  server.send(
    200,
    "application/json",
    json);
}


// ============================================================
// SAVE SETTINGS
// ============================================================

void handleSave() {

  if (
    server.hasArg(
      "start")) {

    String value =
      server.arg(
        "start");


    if (
      value.length() >= 5) {

      settings.startHour =
        value.substring(
               0,
               2)
          .toInt();

      settings.startMinute =
        value.substring(
               3,
               5)
          .toInt();
    }
  }


  if (
    server.hasArg(
      "duration")) {

    settings.durationMinutes =
      server.arg(
              "duration")
        .toInt();
  }


  if (
    server.hasArg(
      "brightness")) {

    settings.maxBrightness =
      server.arg(
              "brightness")
        .toInt();
  }


  if (
    server.hasArg(
      "rampUp")) {

    settings.rampUpMinutes =
      server.arg(
              "rampUp")
        .toInt();
  }


  if (
    server.hasArg(
      "rampDown")) {

    settings.rampDownMinutes =
      server.arg(
              "rampDown")
        .toInt();
  }


  if (
    server.hasArg(
      "siestaEnabled")) {

    settings.siestaEnabled =
      server.arg(
        "siestaEnabled")
      == "1";
  }


  if (
    server.hasArg(
      "siestaStart")) {

    String value =
      server.arg(
        "siestaStart");


    if (
      value.length() >= 5) {

      settings.siestaStartHour =
        value.substring(
               0,
               2)
          .toInt();

      settings.siestaStartMinute =
        value.substring(
               3,
               5)
          .toInt();
    }
  }


  if (
    server.hasArg(
      "siestaEnd")) {

    String value =
      server.arg(
        "siestaEnd");


    if (
      value.length() >= 5) {

      settings.siestaEndHour =
        value.substring(
               0,
               2)
          .toInt();

      settings.siestaEndMinute =
        value.substring(
               3,
               5)
          .toInt();
    }
  }


  if (
    server.hasArg(
      "acclimationEnabled")) {

    settings.acclimationEnabled =
      server.arg(
        "acclimationEnabled")
      == "1";
  }


  if (
    server.hasArg(
      "acclimationStart")) {

    settings.acclimationStartBrightness =
      server.arg(
              "acclimationStart")
        .toInt();
  }


  if (
    server.hasArg(
      "acclimationTarget")) {

    settings.acclimationTargetBrightness =
      server.arg(
              "acclimationTarget")
        .toInt();
  }


  if (
    server.hasArg(
      "acclimationDays")) {

    settings.acclimationDays =
      server.arg(
              "acclimationDays")
        .toInt();
  }


  if (
    server.hasArg(
      "acclimationDay")) {

    settings.acclimationDay =
      server.arg(
              "acclimationDay")
        .toInt();
  }

  if (
    server.hasArg(
      "preset")) {

    String preset =
      server.arg(
        "preset");

    if (preset == "LOW TECH")
      settings.preset = 1;

    else if (preset == "PLANTED")
      settings.preset = 2;

    else if (preset == "BRIGHT PLANTED")
      settings.preset = 3;

    else
      settings.preset = 0;
  }

  /*
     Validate.
  */

  settings.startHour =
    constrain(
      settings.startHour,
      0,
      23);

  settings.startMinute =
    constrain(
      settings.startMinute,
      0,
      59);

  settings.durationMinutes =
    constrain(
      settings.durationMinutes,
      1,
      1440);

  settings.rampUpMinutes =
    constrain(
      settings.rampUpMinutes,
      0,
      120);

  settings.rampDownMinutes =
    constrain(
      settings.rampDownMinutes,
      0,
      120);

  settings.maxBrightness =
    constrain(
      settings.maxBrightness,
      1,
      100);

  settings.siestaStartHour =
    constrain(
      settings.siestaStartHour,
      0,
      23);

  settings.siestaStartMinute =
    constrain(
      settings.siestaStartMinute,
      0,
      59);

  settings.siestaEndHour =
    constrain(
      settings.siestaEndHour,
      0,
      23);

  settings.siestaEndMinute =
    constrain(
      settings.siestaEndMinute,
      0,
      59);

  settings.acclimationStartBrightness =
    constrain(
      settings.acclimationStartBrightness,
      1,
      100);

  settings.acclimationTargetBrightness =
    constrain(
      settings.acclimationTargetBrightness,
      1,
      100);

  settings.acclimationDays =
    constrain(
      settings.acclimationDays,
      1,
      365);

  settings.acclimationDay =
    constrain(
      settings.acclimationDay,
      1,
      settings.acclimationDays);


  bool success =
    saveSettings();


  /*
     Saving settings exits manual mode.
  */

  manualMode =
    false;

  boostMode =
    false;


  updateLight();


  addEvent(
    "Lighting settings saved");


  String json =
    "{\"success\":" + String(success ? "true" : "false") + "}";


  server.send(
    success
      ? 200
      : 500,

    "application/json",

    json);
}


// ============================================================
// MANUAL CONTROL
// ============================================================

void handleManual() {

  if (
    !server.hasArg(
      "brightness")) {

    server.send(
      400,
      "text/plain",
      "Missing brightness");

    return;
  }


  manualBrightness =
    constrain(
      server.arg(
              "brightness")
        .toInt(),
      0,
      100);


  manualMode =
    true;

  boostMode =
    false;


  updateLight();


  addEvent(
    "Manual brightness " + String(manualBrightness) + "%");


  server.send(
    200,
    "text/plain",
    "OK");
}


// ============================================================
// RETURN AUTO
// ============================================================

void handleAuto() {

  manualMode =
    false;


  boostMode =
    false;


  updateLight();


  addEvent(
    "Returned to automatic schedule");


  server.send(
    200,
    "text/plain",
    "OK");
}


// ============================================================
// BOOST
// ============================================================

void handleBoost() {

  if (
    !server.hasArg(
      "brightness")
    || !server.hasArg(
      "minutes")) {

    server.send(
      400,
      "text/plain",
      "Missing arguments");

    return;
  }


  boostBrightness =
    constrain(
      server.arg(
              "brightness")
        .toInt(),
      1,
      100);


  int minutes =
    constrain(
      server.arg(
              "minutes")
        .toInt(),
      1,
      180);


  boostMode =
    true;

  manualMode =
    false;


  boostEndMillis =
    millis() + ((unsigned long)minutes * 60000UL);


  updateLight();


  addEvent(
    "Boost started " + String(boostBrightness) + "% for " + String(minutes) + " min");


  server.send(
    200,
    "text/plain",
    "OK");
}


// ============================================================
// STOP BOOST
// ============================================================

void handleBoostStop() {

  boostMode =
    false;


  updateLight();


  addEvent(
    "Boost stopped");


  server.send(
    200,
    "text/plain",
    "OK");
}


// ============================================================
// EVENTS
// ============================================================

void handleEvents() {

  String json =
    "{\"events\":[";


  /*
     Return oldest -> newest.
  */

  for (
    int i = 0;
    i < EVENT_COUNT;
    i++) {

    int index =
      (eventLogIndex + i) % EVENT_COUNT;


    if (
      eventLog[index].length() == 0)
      continue;


    if (
      json.endsWith(
        "[")
      == false)
      json += ",";


    String value =
      eventLog[index];


    value.replace(
      "\\",
      "\\\\");

    value.replace(
      "\"",
      "\\\"");


    json +=
      "\"" + value + "\"";
  }


  json +=
    "]}";


  server.send(
    200,
    "application/json",
    json);
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
    i++) {

    if (
      i > 0)
      json += ",";


    String name =
      WiFi.SSID(i);


    name.replace(
      "\\",
      "\\\\");

    name.replace(
      "\"",
      "\\\"");


    json +=
      "{\"ssid\":\"" + name + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }


  json +=
    "]";


  WiFi.scanDelete();


  server.send(
    200,
    "application/json",
    json);
}


// ============================================================
// WIFI SAVE
// ============================================================

void handleWiFiSave() {

  String ssid = "";

  String password = "";


  if (
    server.hasArg(
      "manualssid")) {

    ssid =
      server.arg(
        "manualssid");

    ssid.trim();
  }


  if (
    ssid.length() == 0 && server.hasArg("ssid")) {

    ssid =
      server.arg(
        "ssid");

    ssid.trim();
  }


  if (
    server.hasArg(
      "password")) {

    password =
      server.arg(
        "password");
  }


  if (
    ssid.length() == 0) {

    server.send(
      400,
      "text/plain",
      "SSID required");

    return;
  }


  if (
    !saveWiFiCredentials(
      ssid,
      password)) {

    server.send(
      500,
      "text/plain",
      "Save failed");

    return;
  }


  server.send(
    200,
    "text/html",

    "<html>"
    "<body style='background:#000;"
    "color:#00ffe1;"
    "font-family:Arial;"
    "text-align:center;"
    "padding:50px'>"
    "<h2>GXDE</h2>"
    "<p>Wi-Fi saved.</p>"
    "<p>Restarting...</p>"
    "</body>"
    "</html>");


  delay(1000);

  ESP.restart();
}


// ============================================================
// FLASH RESET
// ============================================================

void resetWiFiFromButton() {

  if (
    wifiResetTriggered)
    return;


  wifiResetTriggered =
    true;


  Serial.println(
    "WiFi reset requested.");


  deleteWiFiCredentials();


  Serial.println(
    "Release FLASH button.");


  while (
    digitalRead(
      WIFI_BUTTON_PIN)
    == LOW) {

    delay(20);

    yield();
  }


  delay(500);


  ESP.restart();
}


// ============================================================
// CHECK FLASH
// ============================================================

void checkWiFiButton() {

  int state =
    digitalRead(
      WIFI_BUTTON_PIN);


  if (
    !wifiButtonArmed) {

    if (
      state == HIGH) {

      wifiButtonArmed =
        true;

      wifiButtonPrevious =
        HIGH;
    }

    return;
  }


  if (
    state == LOW && wifiButtonPrevious == HIGH) {

    delay(
      WIFI_BUTTON_DEBOUNCE_MS);


    if (
      digitalRead(
        WIFI_BUTTON_PIN)
      == LOW) {

      wifiButtonPressStart =
        millis();


      Serial.println(
        "FLASH pressed");
    }
  }


  if (
    state == LOW && wifiButtonPressStart != 0) {

    if (
      millis() - wifiButtonPressStart >= WIFI_BUTTON_HOLD_MS) {

      resetWiFiFromButton();

      return;
    }
  }


  if (
    state == HIGH && wifiButtonPrevious == LOW) {

    wifiButtonPressStart =
      0;
  }


  wifiButtonPrevious =
    state;
}


// ============================================================
// WEB SERVER
// ============================================================

void setupWebServer() {

  server.on(
    "/",
    HTTP_GET,
    handleRoot);


  server.on(
    "/status",
    HTTP_GET,
    handleStatus);


  server.on(
    "/settings",
    HTTP_GET,
    handleSettings);


  server.on(
    "/save",
    HTTP_GET,
    handleSave);


  server.on(
    "/manual",
    HTTP_GET,
    handleManual);


  server.on(
    "/auto",
    HTTP_GET,
    handleAuto);


  server.on(
    "/boost",
    HTTP_GET,
    handleBoost);


  server.on(
    "/boostStop",
    HTTP_GET,
    handleBoostStop);


  server.on(
    "/events",
    HTTP_GET,
    handleEvents);


  server.on(
    "/wifi/scan",
    HTTP_GET,
    handleWiFiScan);


  server.on(
    "/wifi/save",
    HTTP_POST,
    handleWiFiSave);


  server.begin();


  Serial.println(
    "Web server started.");
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(
    115200);


  delay(300);


  Serial.println();
  Serial.println(
    "====================================");

  Serial.println(
    "GXDE SMART AQUARIUM LIGHT");

  Serial.println(
    "ESP8266 + DS1302 + MOSFET");

  Serial.println(
    "====================================");


  // ==========================================================
  // TIMEZONE
  // ==========================================================

  setupTimezone();


  // ==========================================================
  // FLASH BUTTON
  // ==========================================================

  pinMode(
    WIFI_BUTTON_PIN,
    INPUT_PULLUP);


  wifiButtonPrevious =
    digitalRead(
      WIFI_BUTTON_PIN);


  if (
    wifiButtonPrevious == HIGH)
    wifiButtonArmed =
      true;


  // ==========================================================
  // PWM
  // ==========================================================

  pinMode(
    PWM_PIN,
    OUTPUT);


  analogWriteRange(
    PWM_MAX);


  analogWriteFreq(
    1000);


  setBrightness(
    0);


  // ==========================================================
  // LITTLEFS
  // ==========================================================

  if (
    !LittleFS.begin()) {

    Serial.println(
      "ERROR: LittleFS failed");

    return;
  }


  // ==========================================================
  // SETTINGS
  // ==========================================================

  loadSettings();


  // ==========================================================
  // RTC
  // ==========================================================

  setupRTC();


  // ==========================================================
  // WIFI
  // ==========================================================

  bool hasWiFi =
    loadWiFiCredentials();


  bool connected =
    false;


  if (
    hasWiFi) {

    connected =
      connectToWiFi();
  }


  if (
    !connected) {

    startAccessPoint();

  } else {

    apMode =
      false;

    startNTP();
  }


  // ==========================================================
  // WEB SERVER
  // ==========================================================

  setupWebServer();


  // ==========================================================
  // LIGHT STARTUP
  // ==========================================================

  manualMode =
    false;

  boostMode =
    false;


  /*
     Important:

     Immediately calculate the correct
     schedule position.

     If power came back during the middle
     of the day, we don't restart sunrise.
  */

  updateLight();


  // ==========================================================
  // READY
  // ==========================================================

  Serial.println();
  Serial.println(
    "====================================");

  Serial.println(
    "GXDE READY");

  Serial.print(
    "Current time: ");

  Serial.println(
    getTimeString());


  Serial.print(
    "Schedule: ");

  Serial.print(
    formatMinutes(
      scheduleStartMinutes()));

  Serial.print(
    " -> ");

  Serial.println(
    formatMinutes(
      scheduleOffMinutes()));


  Serial.print(
    "Maximum brightness: ");

  Serial.print(
    getEffectiveMaximumBrightness());

  Serial.println("%");


  Serial.print(
    "Current brightness: ");

  Serial.print(
    currentBrightness);

  Serial.println("%");


  if (
    apMode) {

    Serial.println();
    Serial.println(
      "AP MODE");

    Serial.print(
      "SSID: ");

    Serial.println(
      AP_NAME);

    Serial.print(
      "Password: ");

    Serial.println(
      AP_PASSWORD);

    Serial.print(
      "IP: ");

    Serial.println(
      WiFi.softAPIP());

  } else {

    Serial.print(
      "IP: ");

    Serial.println(
      WiFi.localIP());
  }


  Serial.println(
    "====================================");
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
  // WiFi
  // ----------------------------------------------------------

  updateWiFi();


  // ----------------------------------------------------------
  // NTP
  // ----------------------------------------------------------

  checkNTP();

  periodicNTP();


  // ----------------------------------------------------------
  // Light update
  //
  // 100ms gives the ramp a smooth response.
  // ----------------------------------------------------------

  if (
    millis() - lastLightUpdate >= 100UL) {

    lastLightUpdate =
      millis();


    updateLight();
  }


  yield();
}
