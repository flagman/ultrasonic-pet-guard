#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <time.h>
#include <rom/gpio.h>             // gpio_matrix_out — routes a peripheral signal onto a GPIO
#include <soc/gpio_sig_map.h>     // LEDC_HS_SIG_OUT0_IDX — LEDC signal index
#include "config.h"               // WiFi credentials (copied from config.example.h, gitignored)

void playUltrasound(int durationMs);
void playTone(uint32_t freqHz, int durationMs);
bool isActiveNow();
void fireBurst();
void loadSchedule();
void saveSchedule();
void handleRoot();
void handleSave();
void handleTest();
void handleStatus();
// ==========================================
// ULTRASONIC BARRIER FOR DOGS (ESP32)
// ==========================================

// Pin assignments
const int pirPin = 27;    // AM312 motion sensor
const int in1Pin = 13;   // L298N IN1 — LEDC, non-inverted
const int in3Pin = 12;   // L298N IN3 — LEDC, inverted (full H-bridge between OUT1 and OUT3)
const int enaPin = 14;    // L298N ENA + ENB (tied together on the module) — shared enable

// LEDC: ESP32 hardware PWM (generates the square wave itself, no jitter from FreeRTOS)
const int ledcChannel = 0;
const int ledcFreqUltrasound = 25000;  // 25 kHz — operating frequency
const int ledcResolution = 8;          // 8-bit: duty 0..255

// --- WiFi / NTP ---
const char* wifiSsid     = WIFI_SSID;      // defined in config.h
const char* wifiPassword = WIFI_PASSWORD;  // defined in config.h
const char* ntpServer    = "pool.ntp.org";
// POSIX TZ for Warsaw: CET (UTC+1) in winter, CEST (UTC+2) in summer.
// M3.5.0 = last Sunday of March, M10.5.0/3 = last Sunday of October at 03:00.
const char* tzPosix      = "CET-1CEST,M3.5.0,M10.5.0/3";

// --- Active operating window (editable through the web UI) ---
uint8_t startHour   = 21;
uint8_t startMinute = 0;
uint8_t endHour     = 7;
uint8_t endMinute   = 0;
bool    scheduleEnabled = true;   // global kill switch
unsigned long lastTriggerMs = 0;  // shown in the UI

// Flag: did the time sync succeed? If not, run 24/7 (fail-safe).
bool timeSynced = false;

// --- Web server + NVS ---
WebServer   server(80);
Preferences prefs;
const char* mdnsHost = "pet-guard";  // → http://pet-guard.local

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=== Ultrasonic Pet Guard ===");
  Serial.printf("PIR: GPIO %d | IN1: GPIO %d | IN3: GPIO %d | ENA: GPIO %d\n",
                pirPin, in1Pin, in3Pin, enaPin);
  Serial.printf("LEDC: ch %d, freq %d Hz, %d-bit\n",
                ledcChannel, ledcFreqUltrasound, ledcResolution);

  // Load the schedule from NVS (defaults: 21:00 → 07:00, enabled).
  loadSchedule();
  Serial.printf("Loaded schedule: %02u:%02u - %02u:%02u, enabled=%s\n",
                startHour, startMinute, endHour, endMinute,
                scheduleEnabled ? "yes" : "no");

  // Pin setup
  pinMode(pirPin, INPUT);
  pinMode(in1Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(enaPin, OUTPUT);

  // Make sure the speaker AND the amplifier are off at boot
  digitalWrite(in1Pin, LOW);
  digitalWrite(in3Pin, LOW);
  digitalWrite(enaPin, LOW); // hard-mute the L298N!

  // LEDC: one channel, two pins driven in antiphase.
  // IN1 gets the direct square wave; IN3 gets the same signal inverted
  // through the ESP32 GPIO matrix. This forms a full H-bridge between OUT1
  // and OUT3, i.e. ±Vsupply across the piezo instead of 0..Vsupply (+6 dB).
  ledcSetup(ledcChannel, ledcFreqUltrasound, ledcResolution);
  ledcAttachPin(in1Pin, ledcChannel);
  gpio_matrix_out(in3Pin, LEDC_HS_SIG_OUT0_IDX + ledcChannel, true, false);
  ledcWrite(ledcChannel, 0); // silent at boot (ENA is LOW anyway)

  // --- WiFi + NTP ---
  Serial.printf("Connecting to WiFi \"%s\"", wifiSsid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi OK, IP: %s, RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());

    configTzTime(tzPosix, ntpServer);
    struct tm tinfo;
    if (getLocalTime(&tinfo, 5000)) {
      timeSynced = true;
      Serial.printf("Time synced: %04d-%02d-%02d %02d:%02d:%02d %s\n",
                    tinfo.tm_year + 1900, tinfo.tm_mon + 1, tinfo.tm_mday,
                    tinfo.tm_hour, tinfo.tm_min, tinfo.tm_sec,
                    tinfo.tm_isdst > 0 ? "CEST" : "CET");
      Serial.printf("Active window: %02u:%02u - %02u:%02u\n",
                    startHour, startMinute, endHour, endMinute);
    } else {
      Serial.println("NTP sync failed — falling back to 24/7 mode");
    }

    // mDNS: open http://pet-guard.local/ from a phone/laptop browser.
    if (MDNS.begin(mdnsHost)) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("mDNS started: http://%s.local/\n", mdnsHost);
    } else {
      Serial.println("mDNS start failed");
    }

    server.on("/",          HTTP_GET,  handleRoot);
    server.on("/save",      HTTP_POST, handleSave);
    server.on("/test",      HTTP_POST, handleTest);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.begin();
    Serial.printf("Web UI: http://%s/\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("WiFi failed — falling back to 24/7 mode, no web UI");
  }

  // Give the motion sensor time to calibrate
  Serial.println("PIR warmup (2s)...");
  delay(2000);
  Serial.println("Ready. Waiting for motion.");
}

// Active operating window (minute resolution, may cross midnight).
// If the time is not synced — always active (fail-safe), but only when scheduleEnabled.
bool isActiveNow() {
  if (!scheduleEnabled) return false;
  if (!timeSynced) return true;
  struct tm tinfo;
  if (!getLocalTime(&tinfo, 50)) return true;

  int nowMins   = tinfo.tm_hour * 60 + tinfo.tm_min;
  int startMins = startHour     * 60 + startMinute;
  int endMins   = endHour       * 60 + endMinute;

  if (startMins == endMins) return false;                 // empty window
  if (startMins <  endMins) return nowMins >= startMins && nowMins < endMins;
  return nowMins >= startMins || nowMins < endMins;       // crosses midnight
}

// Load/save the schedule in NVS (survives reboots).
void loadSchedule() {
  prefs.begin("pet-guard", true);  // read-only
  startHour       = prefs.getUChar("sH", 21);
  startMinute     = prefs.getUChar("sM", 0);
  endHour         = prefs.getUChar("eH", 7);
  endMinute       = prefs.getUChar("eM", 0);
  scheduleEnabled = prefs.getBool("on",  true);
  prefs.end();
}

void saveSchedule() {
  prefs.begin("pet-guard", false); // RW
  prefs.putUChar("sH", startHour);
  prefs.putUChar("sM", startMinute);
  prefs.putUChar("eH", endHour);
  prefs.putUChar("eM", endMinute);
  prefs.putBool ("on", scheduleEnabled);
  prefs.end();
}

// A burst of 10 ultrasonic pulses. Factored out so it can be triggered both
// from loop() and from the "Test" button in the web UI.
void fireBurst() {
  lastTriggerMs = millis();
  for (int i = 0; i < 10; i++) {
    playUltrasound(300);
    delay(150);
  }
}

// ----- Web handlers ---------------------------------------------------------

static String hhmm(uint8_t h, uint8_t m) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u:%02u", h, m);
  return String(buf);
}

void handleRoot() {
  struct tm tinfo;
  bool haveTime = timeSynced && getLocalTime(&tinfo, 50);
  String nowStr = haveTime
    ? String(tinfo.tm_hour < 10 ? "0" : "") + tinfo.tm_hour + ":" +
      String(tinfo.tm_min  < 10 ? "0" : "") + tinfo.tm_min
    : String("--:--");
  bool active = isActiveNow();

  String html;
  html.reserve(2500);
  html += F(
    "<!doctype html><html><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>Pet Guard</title>"
    "<style>"
    "body{font-family:system-ui,sans-serif;max-width:420px;margin:24px auto;"
    "padding:0 16px;color:#1f2937}"
    "h1{font-size:20px;margin:0 0 16px}"
    "fieldset{border:1px solid #e5e7eb;border-radius:8px;padding:12px 16px;"
    "margin:0 0 16px}"
    "legend{font-size:13px;color:#6b7280;padding:0 6px}"
    "label{display:block;margin:10px 0 4px;font-size:14px}"
    "input[type=time]{font-size:18px;padding:6px 8px;width:130px;"
    "border:1px solid #d1d5db;border-radius:6px}"
    "button{padding:10px 16px;font-size:15px;border-radius:6px;border:0;"
    "background:#2563eb;color:#fff;cursor:pointer;width:100%}"
    "button.secondary{background:#64748b;margin-top:12px}"
    ".badge{display:inline-block;padding:6px 10px;border-radius:999px;"
    "font-size:13px;font-weight:600}"
    ".on{background:#dcfce7;color:#166534}"
    ".off{background:#fee2e2;color:#991b1b}"
    ".row{display:flex;align-items:center;gap:8px;margin:6px 0}"
    ".muted{color:#6b7280;font-size:13px;margin-top:8px}"
    "</style></head><body>"
    "<h1>Ultrasonic Pet Guard</h1>"
  );
  html += "<div class=\"row\"><span class=\"badge ";
  html += active ? "on\">ACTIVE" : "off\">IDLE";
  html += "</span><span>Now: <b>" + nowStr + "</b>";
  if (haveTime) html += tinfo.tm_isdst > 0 ? " CEST" : " CET";
  html += "</span></div>";

  html += "<form action=\"/save\" method=\"POST\"><fieldset><legend>Schedule</legend>";
  html += "<label><input type=\"checkbox\" name=\"enabled\"";
  if (scheduleEnabled) html += " checked";
  html += "> Enabled</label>";
  html += "<label>Start time</label><input type=\"time\" name=\"start\" value=\"" + hhmm(startHour, startMinute) + "\">";
  html += "<label>End time</label><input type=\"time\" name=\"end\" value=\"" + hhmm(endHour, endMinute) + "\">";
  html += "<div class=\"muted\">Window may cross midnight (e.g. 21:00 → 07:00).</div>";
  html += "</fieldset><button type=\"submit\">Save</button></form>";

  html += "<form action=\"/test\" method=\"POST\">"
          "<button class=\"secondary\" type=\"submit\">Test (fire burst now)</button></form>";

  html += "<p class=\"muted\">WiFi: " + WiFi.SSID() + " (" + WiFi.RSSI() + " dBm)<br>";
  html += "IP: " + WiFi.localIP().toString() + "<br>";
  html += "Uptime: " + String(millis() / 1000) + " s</p>";
  html += "</body></html>";

  server.send(200, "text/html; charset=utf-8", html);
}

static bool parseHHMM(const String& s, uint8_t& h, uint8_t& m) {
  int colon = s.indexOf(':');
  if (colon < 1) return false;
  int hh = s.substring(0, colon).toInt();
  int mm = s.substring(colon + 1).toInt();
  if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return false;
  h = (uint8_t)hh;
  m = (uint8_t)mm;
  return true;
}

void handleSave() {
  uint8_t sH, sM, eH, eM;
  if (!parseHHMM(server.arg("start"), sH, sM) ||
      !parseHHMM(server.arg("end"),   eH, eM)) {
    server.send(400, "text/plain", "Bad time format");
    return;
  }
  startHour       = sH;
  startMinute     = sM;
  endHour         = eH;
  endMinute       = eM;
  scheduleEnabled = server.hasArg("enabled");
  saveSchedule();

  Serial.printf("Schedule updated: %s -> %s, enabled=%s\n",
                hhmm(startHour, startMinute).c_str(),
                hhmm(endHour,   endMinute).c_str(),
                scheduleEnabled ? "yes" : "no");

  server.sendHeader("Location", "/", true);
  server.send(303);
}

void handleTest() {
  Serial.println("Manual test fire (web)");
  fireBurst();
  server.sendHeader("Location", "/", true);
  server.send(303);
}

void handleStatus() {
  struct tm tinfo;
  bool haveTime = timeSynced && getLocalTime(&tinfo, 50);
  String json = "{";
  json += "\"enabled\":";        json += scheduleEnabled ? "true" : "false";
  json += ",\"active\":";        json += isActiveNow()    ? "true" : "false";
  json += ",\"start\":\"" + hhmm(startHour, startMinute) + "\"";
  json += ",\"end\":\""   + hhmm(endHour,   endMinute)   + "\"";
  json += ",\"now\":\"";
  if (haveTime) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
             tinfo.tm_year + 1900, tinfo.tm_mon + 1, tinfo.tm_mday,
             tinfo.tm_hour, tinfo.tm_min, tinfo.tm_sec);
    json += buf;
  }
  json += "\",\"uptimeMs\":";   json += millis();
  json += ",\"lastTriggerMs\":"; json += lastTriggerMs;
  json += ",\"rssi\":";          json += WiFi.RSSI();
  json += "}";
  server.send(200, "application/json", json);
}

void loop() {
  server.handleClient();  // service HTTP requests (no-op if WiFi never came up)

  static int lastPirState = LOW;
  int pirState = digitalRead(pirPin);

  if (pirState != lastPirState) {
    Serial.printf("[%lu ms] PIR -> %s\n", millis(), pirState ? "HIGH" : "LOW");
    lastPirState = pirState;
  }

  if (pirState == HIGH) {
    if (!isActiveNow()) {
      struct tm tinfo;
      if (getLocalTime(&tinfo, 50)) {
        Serial.printf("[%02d:%02d] Motion ignored (window %02u:%02u-%02u:%02u, enabled=%s)\n",
                      tinfo.tm_hour, tinfo.tm_min,
                      startHour, startMinute, endHour, endMinute,
                      scheduleEnabled ? "yes" : "no");
      }
      delay(1000); // light debounce so the log isn't flooded
      return;
    }

    Serial.println("Motion! Firing ultrasound burst (10 x 300ms)");
    fireBurst();

    Serial.println("Cooldown 5s...");
    // 5 s cooldown, but keep answering HTTP (sliced into 50 ms delays)
    unsigned long until = millis() + 5000;
    while ((long)(millis() - until) < 0) {
      server.handleClient();
      delay(50);
    }
    Serial.println("Ready.");
  }
}

// -------------------------------------------------------------------
// ULTRASOUND generation (25 kHz) — inaudible to humans
// -------------------------------------------------------------------
void playUltrasound(int durationMs) {
  playTone(ledcFreqUltrasound, durationMs);
}

// -------------------------------------------------------------------
// Shared helper: plays a square wave of the given frequency in full H-bridge.
// The LEDC routing (IN1 direct, IN3 inverted) is configured once in setup().
// Here we only change the frequency, start the PWM, and toggle ENA.
// -------------------------------------------------------------------
void playTone(uint32_t freqHz, int durationMs) {
  ledcSetup(ledcChannel, freqHz, ledcResolution); // re-tune the channel frequency
  ledcWrite(ledcChannel, 128);                     // 50% duty — clean square wave
  digitalWrite(enaPin, HIGH);                      // H-bridge outputs go active

  delay(durationMs);

  // ORDER MATTERS: mute the H-bridge first (outputs → HIGH-Z), and only then
  // stop the LEDC. Otherwise at duty=0 in1Pin sits LOW while in3Pin (inverted)
  // sits HIGH → a DC level across the piezo → a loud click.
  digitalWrite(enaPin, LOW);
  ledcWrite(ledcChannel, 0);
}
