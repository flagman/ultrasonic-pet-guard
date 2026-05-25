#pragma once
// -----------------------------------------------------------------------------
// Ultrasonic Pet Guard — local configuration template.
//
// Copy this file to `config.h` (in the same folder) and fill in your own
// WiFi credentials. `config.h` is listed in .gitignore so your secrets are
// never committed to the repository.
//
//     cp include/config.example.h include/config.h
//
// WiFi is optional: it is only used for NTP time sync (the active-hours
// schedule) and the web UI. If WiFi fails to connect, the firmware falls back
// to running 24/7 with no web UI.
// -----------------------------------------------------------------------------

#define WIFI_SSID     "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"
