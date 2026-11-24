// Registreringsbryter – firmware for ESP32-C3 SuperMini
//
// Fire knapper, hver med sin LED. Trykk = start/stopp tidsregistrering.
// Hendelser lagres først lokalt (LittleFS) og lastes så opp til et GitHub-repo
// som én JSON-linje per hendelse i data/ÅÅÅÅ/MM/ÅÅÅÅ-MM-DD.jsonl.
//
// Biblioteker (Arduino IDE: Verktøy > Behandle biblioteker):
//   - WiFiManager av tzapu (2.0.17 eller nyere)
//   - ArduinoJson av Benoit Blanchon (7.x)
// Kort: "ESP32C3 Dev Module", med "USB CDC On Boot: Enabled".

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include "mbedtls/base64.h"

#include "innstillinger.h"
#include "github_ca.h"

// ---------------------------------------------------------------------------
// Filer i ESP-ens eget minne
// ---------------------------------------------------------------------------
static const char* OPPSETT_FIL  = "/oppsett.json";   // GitHub-eier, repo, gren, token
static const char* TILSTAND_FIL = "/tilstand.json";  // hvilke knapper som er på + siste hjerteslag
static const char* KO_FIL       = "/ko.txt";         // hendelser som ikke er lastet opp ennå
static const char* JSON_ACCEPT  = "application/vnd.github+json";

// ---------------------------------------------------------------------------
// Tilstand
// ---------------------------------------------------------------------------
struct Oppsett {
  String eier;
  String repo;
  String gren = "main";
  String token;
  bool komplett() const { return eier.length() && repo.length() && token.length(); }
};

struct Ventende {          // trykk før klokka er synkronisert
  uint8_t knapp;
  bool start;
  uint32_t ms;
  const char* kilde;
};

Oppsett oppsett;
SemaphoreHandle_t filLaas;
SemaphoreHandle_t oppsettLaas;

WiFiManager wm;
WiFiManagerParameter* pEier;
WiFiManagerParameter* pRepo;
WiFiManagerParameter* pGren;
WiFiManagerParameter* pToken;

bool knappPaa[ANTALL_KNAPPER] = {false};
bool raa[ANTALL_KNAPPER];
bool stabil[ANTALL_KNAPPER];
uint32_t endretMs[ANTALL_KNAPPER];

std::vector<Ventende> ventende;
uint32_t sistHjerteslagMs = 0;
uint32_t bootHoldMs = 0;
bool wifiHarVaertTilkoblet = false;
uint32_t wifiForsokStartMs = 0;
bool portalVarAktiv = false;

volatile bool koHarInnhold = false;
volatile uint32_t koSidenMs = 0;
volatile uint32_t sistKoEndringMs = 0;
volatile int autoAvMin = -1;          // minutter etter midnatt, -1 = av
volatile bool oppsettEndret = true;   // be nettverksoppgaven hente config på nytt

// Enkel lås som slipper automatisk når den går ut av scope
struct Laas {
  SemaphoreHandle_t s;
  explicit Laas(SemaphoreHandle_t x) : s(x) { xSemaphoreTake(s, portMAX_DELAY); }
  ~Laas() { xSemaphoreGive(s); }
};

// ---------------------------------------------------------------------------
// Tid
// ---------------------------------------------------------------------------
bool tidGyldig() { return time(nullptr) > 1700000000; }

String isoUtc(time_t t) {
  struct tm u;
  gmtime_r(&t, &u);
  char b[24];
  strftime(b, sizeof b, "%Y-%m-%dT%H:%M:%SZ", &u);
  return String(b);
}

String lokalDato(time_t t) {
  struct tm l;
  localtime_r(&t, &l);
  char b[12];
  strftime(b, sizeof b, "%Y-%m-%d", &l);
  return String(b);
}

int lesKlokkeslett(const char* s) {  // "HH:MM" -> minutter, ellers -1
  int h, m;
  if (!s || sscanf(s, "%d:%d", &h, &m) != 2) return -1;
  if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
  return h * 60 + m;
}

// ---------------------------------------------------------------------------
// Lagring av oppsett
// ---------------------------------------------------------------------------
void lesOppsett() {
  Laas l(filLaas);
  File f = LittleFS.open(OPPSETT_FIL, "r");
  if (!f) return;
  JsonDocument d;
  if (!deserializeJson(d, f)) {
    Laas o(oppsettLaas);
    oppsett.eier  = d["eier"]  | "";
    oppsett.repo  = d["repo"]  | "";
    oppsett.gren  = d["gren"]  | "main";
    oppsett.token = d["token"] | "";
  }
  f.close();
}

void lagreOppsett() {
  JsonDocument d;
  {
    Laas o(oppsettLaas);
    d["eier"] = oppsett.eier;
    d["repo"] = oppsett.repo;
    d["gren"] = oppsett.gren;
    d["token"] = oppsett.token;
  }
  Laas l(filLaas);
  File f = LittleFS.open(OPPSETT_FIL, "w");
  if (f) {
    serializeJson(d, f);
    f.close();
  }
}

bool oppsettKomplett() {
  Laas o(oppsettLaas);
  return oppsett.komplett();
}

// ---------------------------------------------------------------------------
// Hendelseskø
// ---------------------------------------------------------------------------
// Hver linje i køen: "ÅÅÅÅ-MM-DD|{json}". Datoen er lokal dato og bestemmer filnavnet.
bool gyldigKoLinje(const String& s) { return s.length() > 12 && s.charAt(10) == '|'; }

void leggIKo(uint8_t knapp, bool start, time_t t, const char* kilde) {
  JsonDocument d;
  d["tid"] = isoUtc(t);
  d["knapp"] = knapp;
  d["hendelse"] = start ? "start" : "stopp";
  d["kilde"] = kilde;
  String json;
  serializeJson(d, json);
  String linje = lokalDato(t) + "|" + json;
  {
    Laas l(filLaas);
    File f = LittleFS.open(KO_FIL, FILE_APPEND);
    if (f) {
      f.print(linje);
      f.print('\n');
      f.close();
    }
  }
  if (!koHarInnhold) koSidenMs = millis();
  koHarInnhold = true;
  sistKoEndringMs = millis();
  Serial.printf("[hendelse] %s\n", json.c_str());
}

// ---------------------------------------------------------------------------
// Tilstand (overlever strømbrudd)
// ---------------------------------------------------------------------------
void lagreTilstand() {
  JsonDocument d;
  JsonArray a = d["paa"].to<JsonArray>();
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) a.add(knappPaa[i] ? 1 : 0);
  d["hjerteslag"] = tidGyldig() ? (int64_t)time(nullptr) : (int64_t)0;
  Laas l(filLaas);
  File f = LittleFS.open(TILSTAND_FIL, "w");
  if (f) {
    serializeJson(d, f);
    f.close();
  }
  sistHjerteslagMs = millis();
}

// Var noen knapper på da strømmen forsvant? Da registreres stopp ved siste
// hjerteslag (maks 2 min unøyaktighet), og alle knapper starter som av.
void gjenopprettEtterStrombrudd() {
  JsonDocument d;
  {
    Laas l(filLaas);
    File f = LittleFS.open(TILSTAND_FIL, "r");
    if (!f) return;
    DeserializationError e = deserializeJson(d, f);
    f.close();
    if (e) return;
  }
  int64_t hjerteslag = d["hjerteslag"] | (int64_t)0;
  JsonArray a = d["paa"];
  for (uint8_t i = 0; i < ANTALL_KNAPPER && i < a.size(); i++) {
    if (a[i].as<int>() == 1 && hjerteslag > 0) {
      leggIKo(i + 1, false, (time_t)hjerteslag, "strombrudd");
    }
  }
}

// ---------------------------------------------------------------------------
// Knapper og LED-er
// ---------------------------------------------------------------------------
bool noenPaa() {
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++)
    if (knappPaa[i]) return true;
  return false;
}

void registrer(uint8_t i, bool start, const char* kilde) {
  if (tidGyldig()) {
    leggIKo(i + 1, start, time(nullptr), kilde);
  } else {
    ventende.push_back({uint8_t(i + 1), start, millis(), kilde});
    Serial.printf("[hendelse] knapp %u %s (venter på klokka)\n", i + 1, start ? "start" : "stopp");
  }
}

void veksle(uint8_t i) {
  knappPaa[i] = !knappPaa[i];
  registrer(i, knappPaa[i], "knapp");
  lagreTilstand();
}

void stoppAlle(const char* kilde) {
  bool endret = false;
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    if (knappPaa[i]) {
      knappPaa[i] = false;
      registrer(i, false, kilde);
      endret = true;
    }
  }
  if (endret) lagreTilstand();
}

void lesKnapper() {
  uint32_t n = millis();
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    bool r = digitalRead(KNAPP_PINNER[i]) == LOW;
    if (r != raa[i]) {
      raa[i] = r;
      endretMs[i] = n;
    } else if (r != stabil[i] && n - endretMs[i] >= DEBOUNCE_MS) {
      stabil[i] = r;
      if (r) veksle(i);  // reagerer når knappen trykkes ned
    }
  }
}

// Trykk som skjedde før klokka var synket får riktig tidspunkt i ettertid.
void tomVentende() {
  if (ventende.empty() || !tidGyldig()) return;
  time_t naa = time(nullptr);
  uint32_t m = millis();
  for (auto& v : ventende) leggIKo(v.knapp, v.start, naa - (time_t)((m - v.ms) / 1000), v.kilde);
  ventende.clear();
  lagreTilstand();
}

void hjerteslag() {
  if (noenPaa() && tidGyldig() && millis() - sistHjerteslagMs >= HJERTESLAG_MS) lagreTilstand();
}

void sjekkAutoAv() {
  static int forrige = -1;
  static uint32_t sist = 0;
  if (millis() - sist < 1000) return;
  sist = millis();
  if (!tidGyldig()) return;
  time_t t = time(nullptr);
  struct tm l;
  localtime_r(&t, &l);
  int m = l.tm_hour * 60 + l.tm_min;
  int a = autoAvMin;
  if (forrige >= 0 && a >= 0 && m != forrige) {
    bool krysset = (forrige < a && m >= a) || (m < forrige && (a > forrige || a <= m));
    if (krysset && noenPaa()) {
      Serial.println("[auto] Skrur av alle knapper (autoAv)");
      stoppAlle("auto");
    }
  }
  forrige = m;
}

// LED-ene viser tilstanden til knappene, men låner dem også til enkle signaler.
void tegnLeder() {
  uint32_t n = millis();
  bool problem = (koHarInnhold && n - koSidenMs > 10UL * 60 * 1000) ||
                 (!ventende.empty() && n - ventende[0].ms > 10UL * 60 * 1000);
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    bool paa = knappPaa[i];
    if (bootHoldMs >= BOOT_HOLD_NULLSTILL_MS) paa = (n / 100) % 2;           // blinker raskt: slipp for å nullstille
    else if (bootHoldMs >= BOOT_HOLD_PORTAL_MS) paa = true;                  // alle på: slipp for oppsettmodus
    else if (wm.getConfigPortalActive()) paa = paa || ((n / 250) % ANTALL_KNAPPER == i);  // løpelys
    else if (problem) paa = paa || (n % 5000) < 80;                          // kort blink hvert 5. sekund
    digitalWrite(LED_PINNER[i], paa ? HIGH : LOW);
  }
  bool status;
  if (wm.getConfigPortalActive()) status = (n / 150) % 2;
  else if (WiFi.status() != WL_CONNECTED) status = (n / 1000) % 2;
  else status = problem && (n % 2000) < 100;
  digitalWrite(STATUS_LED_PIN, status ? LOW : HIGH);  // aktiv lav
}

// ---------------------------------------------------------------------------
// WiFi og oppsettmodus (WiFiManager)
// ---------------------------------------------------------------------------
void lagreParametre() {
  String eier = pEier->getValue();
  String repo = pRepo->getValue();
  String gren = pGren->getValue();
  String token = pToken->getValue();
  eier.trim(); repo.trim(); gren.trim(); token.trim();

  // Godta også "eier/repo" eller hele lenken "https://github.com/eier/repo"
  repo.replace("https://", "");
  repo.replace("github.com/", "");
  if (repo.endsWith(".git")) repo.remove(repo.length() - 4);
  int skille = repo.indexOf('/');
  if (skille > 0) {
    eier = repo.substring(0, skille);
    repo = repo.substring(skille + 1);
  }
  {
    Laas o(oppsettLaas);
    if (eier.length()) oppsett.eier = eier;
    if (repo.length()) oppsett.repo = repo;
    if (gren.length()) oppsett.gren = gren;
    if (token.length()) oppsett.token = token;
  }
  lagreOppsett();
  oppsettEndret = true;
  Serial.println("[oppsett] Lagret GitHub-innstillinger");
}

void settOppPortal() {
  pEier  = new WiFiManagerParameter("eier", "GitHub-bruker eller organisasjon", "", 60);
  pRepo  = new WiFiManagerParameter("repo", "Data-repo (navn eller lenke)", "", 120);
  pGren  = new WiFiManagerParameter("gren", "Gren", "main", 60);
  pToken = new WiFiManagerParameter("token", "GitHub-token (tomt = behold lagret)", "", 200,
                                    "type='password' autocomplete='off'");
  wm.addParameter(pEier);
  wm.addParameter(pRepo);
  wm.addParameter(pGren);
  wm.addParameter(pToken);
  wm.setSaveParamsCallback(lagreParametre);
  wm.setConfigPortalBlocking(false);
  wm.setConnectTimeout(20);
  wm.setWiFiAutoReconnect(true);
  wm.setTitle("Registreringsbryter");
  std::vector<const char*> meny = {"wifi", "info", "restart", "exit"};
  wm.setMenu(meny);
}

void apnePortal(bool medTimeout) {
  {
    Laas o(oppsettLaas);
    pEier->setValue(oppsett.eier.c_str(), 60);
    pRepo->setValue(oppsett.repo.c_str(), 120);
    pGren->setValue(oppsett.gren.c_str(), 60);
  }
  pToken->setValue("", 200);
  wm.setConfigPortalTimeout(medTimeout ? PORTAL_TIMEOUT_S : 0);
  Serial.printf("[wifi] Oppsettmodus: koble til WiFi-nettet \"%s\" og åpne 192.168.4.1\n", PORTAL_NAVN);
  wm.startConfigPortal(PORTAL_NAVN);
}

void passPaaWifi() {
  wm.process();
  bool portal = wm.getConfigPortalActive();
  if (portalVarAktiv && !portal) {
    // Oppsettmodus lukket (lagret eller timeout) – fortsett som vanlig klient
    WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) WiFi.begin();
    wifiForsokStartMs = millis();
  }
  portalVarAktiv = portal;
  if (portal) return;

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiHarVaertTilkoblet) Serial.printf("[wifi] Tilkoblet, IP %s\n", WiFi.localIP().toString().c_str());
    wifiHarVaertTilkoblet = true;
    return;
  }
  // Har ikke fått kontakt siden oppstart (nytt sted / feil passord?): åpne oppsett.
  if (!wifiHarVaertTilkoblet && millis() - wifiForsokStartMs > 60000) {
    apnePortal(true);
    wifiForsokStartMs = millis();
  }
}

// Hold BOOT-knappen på kortet: 3 s = oppsettmodus, 10 s = slett alt.
void sjekkBootKnapp() {
  static uint32_t nedeFra = 0;
  bool nede = digitalRead(BOOT_KNAPP_PIN) == LOW;
  if (nede) {
    if (!nedeFra) nedeFra = millis();
    bootHoldMs = millis() - nedeFra;
    return;
  }
  if (!nedeFra) return;
  uint32_t holdt = bootHoldMs;
  nedeFra = 0;
  bootHoldMs = 0;
  if (holdt >= BOOT_HOLD_NULLSTILL_MS) {
    Serial.println("[oppsett] Nullstiller alle innstillinger");
    stoppAlle("nullstilling");
    wm.resetSettings();
    {
      Laas l(filLaas);
      LittleFS.remove(OPPSETT_FIL);
    }
    delay(500);
    ESP.restart();
  } else if (holdt >= BOOT_HOLD_PORTAL_MS) {
    apnePortal(true);
  }
}

// ---------------------------------------------------------------------------
// GitHub
// ---------------------------------------------------------------------------
bool base64Kod(const String& inn, String& ut) {
  size_t n = 0;
  mbedtls_base64_encode(nullptr, 0, &n, (const unsigned char*)inn.c_str(), inn.length());
  std::vector<unsigned char> buf(n + 1, 0);
  if (mbedtls_base64_encode(buf.data(), buf.size(), &n, (const unsigned char*)inn.c_str(), inn.length()) != 0)
    return false;
  buf[n] = 0;
  ut = (const char*)buf.data();
  return true;
}

bool base64Dekod(String inn, String& ut) {
  inn.replace("\n", "");
  inn.replace("\r", "");
  size_t n = 0;
  mbedtls_base64_decode(nullptr, 0, &n, (const unsigned char*)inn.c_str(), inn.length());
  std::vector<unsigned char> buf(n + 1, 0);
  if (mbedtls_base64_decode(buf.data(), buf.size(), &n, (const unsigned char*)inn.c_str(), inn.length()) != 0)
    return false;
  buf[n] = 0;
  ut = (const char*)buf.data();
  return true;
}

int github(const Oppsett& o, const char* metode, const String& sti, const String& kropp, String& svar,
           const char* accept) {
  WiFiClientSecure klient;
  klient.setCACert(GITHUB_ROTSERTIFIKATER);
  HTTPClient http;
  http.setTimeout(15000);
  http.setReuse(false);
  String url = "https://api.github.com/repos/" + o.eier + "/" + o.repo + "/" + sti;
  if (!http.begin(klient, url)) return -1;
  http.addHeader("Authorization", "Bearer " + o.token);
  http.addHeader("Accept", accept);
  http.addHeader("X-GitHub-Api-Version", "2022-11-28");
  http.setUserAgent(String("Registreringsbryter/") + FIRMWARE_VERSJON);
  int kode;
  if (strcmp(metode, "GET") == 0) {
    kode = http.GET();
  } else {
    http.addHeader("Content-Type", "application/json");
    kode = http.sendRequest(metode, kropp);
  }
  svar = kode > 0 ? http.getString() : String();
  http.end();
  return kode;
}

void hentConfig(const Oppsett& o) {
  String svar;
  int k = github(o, "GET", "contents/config.json?ref=" + o.gren, "", svar, "application/vnd.github.raw+json");
  if (k == 200) {
    JsonDocument d;
    if (!deserializeJson(d, svar)) {
      autoAvMin = lesKlokkeslett(d["autoAv"] | "");
      Serial.printf("[config] autoAv = %s\n", autoAvMin >= 0 ? (const char*)(d["autoAv"] | "") : "av");
    }
  } else if (k == 404) {
    autoAvMin = -1;
    Serial.println("[config] Fant ingen config.json i repoet");
  } else {
    Serial.printf("[config] Klarte ikke å hente config.json (HTTP %d)\n", k);
  }
}

// Laster opp alle køede hendelser for én dag i én commit.
bool lastOppEnDag(const Oppsett& o) {
  std::vector<String> linjer;
  {
    Laas l(filLaas);
    File f = LittleFS.open(KO_FIL, "r");
    if (f) {
      while (f.available()) {
        String s = f.readStringUntil('\n');
        s.trim();
        if (gyldigKoLinje(s)) linjer.push_back(s);
      }
      f.close();
    }
    if (linjer.empty()) {
      LittleFS.remove(KO_FIL);
      koHarInnhold = false;
      return true;
    }
  }

  String dato = linjer[0].substring(0, 10);
  String tillegg;
  std::vector<String> sendt;
  for (auto& s : linjer) {
    if (s.startsWith(dato)) {
      tillegg += s.substring(11) + "\n";
      sendt.push_back(s);
    }
  }

  String sti = String("contents/") + DATA_MAPPE + "/" + dato.substring(0, 4) + "/" + dato.substring(5, 7) + "/" +
               dato + ".jsonl";
  String sha, innhold;
  {
    String svar;
    int k = github(o, "GET", sti + "?ref=" + o.gren, "", svar, JSON_ACCEPT);
    if (k == 200) {
      JsonDocument d;
      if (deserializeJson(d, svar)) return false;
      sha = d["sha"].as<String>();
      if (!base64Dekod(d["content"].as<String>(), innhold)) return false;
    } else if (k != 404) {
      Serial.printf("[github] Lesing av %s feilet (HTTP %d) %s\n", sti.c_str(), k, svar.substring(0, 200).c_str());
      return false;
    }
  }
  if (innhold.length() && !innhold.endsWith("\n")) innhold += "\n";
  innhold += tillegg;

  String kropp;
  {
    String b64;
    if (!base64Kod(innhold, b64)) return false;
    JsonDocument b;
    b["message"] = String("Registrer ") + sendt.size() + " hendelse(r) " + dato;
    b["content"] = b64;
    b["branch"] = o.gren;
    if (sha.length()) b["sha"] = sha;
    serializeJson(b, kropp);
  }
  String svar;
  int k = github(o, "PUT", sti, kropp, svar, JSON_ACCEPT);
  if (k != 200 && k != 201) {
    Serial.printf("[github] Opplasting feilet (HTTP %d) %s\n", k, svar.substring(0, 200).c_str());
    return false;
  }
  Serial.printf("[github] Lastet opp %u hendelse(r) til %s\n", (unsigned)sendt.size(), sti.c_str());

  // Fjern det som er sendt. Nye linjer kan ha kommet til i mellomtiden, så les fila på nytt.
  {
    Laas l(filLaas);
    std::vector<String> alle;
    File f = LittleFS.open(KO_FIL, "r");
    if (f) {
      while (f.available()) {
        String s = f.readStringUntil('\n');
        s.trim();
        if (gyldigKoLinje(s)) alle.push_back(s);
      }
      f.close();
    }
    std::vector<bool> brukt(sendt.size(), false);
    String rest;
    for (auto& s : alle) {
      bool fjern = false;
      for (size_t j = 0; j < sendt.size(); j++) {
        if (!brukt[j] && sendt[j] == s) {
          brukt[j] = true;
          fjern = true;
          break;
        }
      }
      if (!fjern) rest += s + "\n";
    }
    if (rest.length() == 0) {
      LittleFS.remove(KO_FIL);
      koHarInnhold = false;
    } else {
      File w = LittleFS.open(KO_FIL, "w");
      if (w) {
        w.print(rest);
        w.close();
      }
      koSidenMs = millis();
    }
  }
  return true;
}

// Kjører i egen oppgave slik at knappene svarer også mens den snakker med GitHub.
void nettverkOppgave(void*) {
  uint32_t nesteConfigMs = 0;
  bool hentConfigNa = true;
  uint32_t nesteForsokMs = 0;
  uint32_t vent = 5000;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(500));
    if (WiFi.status() != WL_CONNECTED || !tidGyldig()) continue;
    Oppsett o;
    {
      Laas l(oppsettLaas);
      o = oppsett;
    }
    if (!o.komplett()) continue;

    uint32_t n = millis();
    if (oppsettEndret) {
      oppsettEndret = false;
      hentConfigNa = true;
    }
    if (hentConfigNa || (int32_t)(n - nesteConfigMs) >= 0) {
      hentConfig(o);
      hentConfigNa = false;
      nesteConfigMs = millis() + CONFIG_INTERVALL_MS;
    }
    if (koHarInnhold && n - sistKoEndringMs >= SAMLE_TRYKK_MS && (int32_t)(n - nesteForsokMs) >= 0) {
      if (lastOppEnDag(o)) {
        vent = 5000;
        nesteForsokMs = millis();
      } else {
        nesteForsokMs = millis() + vent;
        vent = min<uint32_t>(vent * 2, MAKS_VENTETID_MS);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Oppstart og hovedløkke
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);  // ikke heng når ingen PC leser seriellporten
#endif
  setenv("TZ", TIDSSONE, 1);
  tzset();

  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    pinMode(KNAPP_PINNER[i], INPUT_PULLUP);
    pinMode(LED_PINNER[i], OUTPUT);
    digitalWrite(LED_PINNER[i], LOW);
  }
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  pinMode(BOOT_KNAPP_PIN, INPUT_PULLUP);

  // LED-test ved oppstart
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    digitalWrite(LED_PINNER[i], HIGH);
    delay(150);
    digitalWrite(LED_PINNER[i], LOW);
  }
  Serial.printf("\nRegistreringsbryter %s\n", FIRMWARE_VERSJON);

  filLaas = xSemaphoreCreateMutex();
  oppsettLaas = xSemaphoreCreateMutex();
  if (!LittleFS.begin(true)) Serial.println("[fs] Klarte ikke å starte LittleFS");

  lesOppsett();
  koHarInnhold = LittleFS.exists(KO_FIL);
  koSidenMs = millis();
  gjenopprettEtterStrombrudd();
  lagreTilstand();

  // Start med knappenes faktiske posisjon, så ingenting veksler av seg selv
  for (uint8_t i = 0; i < ANTALL_KNAPPER; i++) {
    raa[i] = stabil[i] = digitalRead(KNAPP_PINNER[i]) == LOW;
    endretMs[i] = millis();
  }

  // Mange SuperMini-kort har svak antenne og kobler ikke til med full effekt.
  WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t) { WiFi.setTxPower(WIFI_POWER_8_5dBm); },
               ARDUINO_EVENT_WIFI_STA_START);
  WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t) { WiFi.setTxPower(WIFI_POWER_8_5dBm); },
               ARDUINO_EVENT_WIFI_AP_START);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  configTzTime(TIDSSONE, NTP_SERVER_1, NTP_SERVER_2);

  settOppPortal();
  if (!wm.getWiFiIsSaved() || !oppsettKomplett()) {
    apnePortal(false);  // står åpen til oppsettet er fullført
  } else {
    WiFi.begin();
    wifiForsokStartMs = millis();
  }

  xTaskCreate(nettverkOppgave, "nettverk", 16384, nullptr, 1, nullptr);
}

void loop() {
  passPaaWifi();
  lesKnapper();
  sjekkBootKnapp();
  tomVentende();
  hjerteslag();
  sjekkAutoAv();
  tegnLeder();
  delay(5);
}
