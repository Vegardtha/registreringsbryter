// Innstillinger for Registreringsbryter.
// Endre pinner her hvis du kobler annerledes enn i README.
#pragma once
#include <Arduino.h>

constexpr const char* FIRMWARE_VERSJON = "1.0.0";

// --- Pinner (ESP32-C3 SuperMini) -------------------------------------------
// Unngå GPIO 2, 8 og 9 (strapping-pinner) til knapper og LED-er.
constexpr uint8_t ANTALL_KNAPPER = 4;
constexpr uint8_t KNAPP_PINNER[ANTALL_KNAPPER] = {0, 1, 3, 4};   // knapp -> GND
constexpr uint8_t LED_PINNER[ANTALL_KNAPPER]   = {5, 6, 7, 10};  // GPIO -> 220Ω -> LED -> GND
constexpr uint8_t STATUS_LED_PIN = 8;   // innebygd blå LED på kortet (aktiv lav)
constexpr uint8_t BOOT_KNAPP_PIN = 9;   // BOOT-knappen på kortet

// --- Tid ----------------------------------------------------------------------
// Norsk tid med sommertid. Brukes til filnavn (én fil per dag) og auto-av.
constexpr const char* TIDSSONE = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr const char* NTP_SERVER_1 = "pool.ntp.org";
constexpr const char* NTP_SERVER_2 = "time.google.com";

// --- Oppførsel ----------------------------------------------------------------
constexpr const char* PORTAL_NAVN = "Registreringsbryter";  // WiFi-navn i oppsettmodus
constexpr uint32_t DEBOUNCE_MS = 40;
constexpr uint32_t HJERTESLAG_MS = 2UL * 60 * 1000;      // lagrer "fortsatt på" hvert 2. min
constexpr uint32_t CONFIG_INTERVALL_MS = 15UL * 60 * 1000; // henter config.json hvert 15. min
constexpr uint32_t SAMLE_TRYKK_MS = 5000;                 // venter litt så raske trykk havner i én commit
constexpr uint32_t MAKS_VENTETID_MS = 5UL * 60 * 1000;    // lengste pause mellom nye forsøk ved feil
constexpr uint32_t PORTAL_TIMEOUT_S = 180;                // oppsettmodus lukkes etter 3 min hvis WiFi er lagret
constexpr uint32_t BOOT_HOLD_PORTAL_MS = 3000;            // hold BOOT 3 s: åpne oppsettmodus
constexpr uint32_t BOOT_HOLD_NULLSTILL_MS = 10000;        // hold BOOT 10 s: slett alle innstillinger

// Mappe i data-repoet der hendelser lagres (data/ÅÅÅÅ/MM/ÅÅÅÅ-MM-DD.jsonl)
constexpr const char* DATA_MAPPE = "data";
