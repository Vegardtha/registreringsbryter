# Registreringsbryter

En boks med fire knapper for timeføring. Trykk på en knapp for å starte tiden på et prosjekt. LED-en ved siden av knappen lyser så lenge den er på. Trykk igjen for å stoppe.

- Flere knapper kan være på samtidig. Hvert prosjekt får da full tid.
- Boksen sender hvert trykk til et privat GitHub-repo. Du trenger ingen server.
- Nettsiden i `web/` viser tiden per dag, uke, måned og år. Der bytter du også hvilket prosjekt hver knapp hører til.

```
 [Boks]  ──WiFi──▶  [GitHub: data-repo]  ◀──  [web/index.html]
 trykk              config.json               oversikt + innstillinger
                    data/2026/11/2026-11-24.jsonl
```

## Innhold

| Mappe | Hva |
|---|---|
| `firmware/` | Koden til ESP32-C3. Fungerer med Arduino IDE eller PlatformIO. |
| `web/` | Nettsiden, én enkelt HTML-fil |
| `data-repo-mal/` | Startfiler til data-repoet |
| `kabinett/` | Spesifikasjon for 3D-printet boks, laget for at Claude skal kunne tegne den |

---

## 1. Deler

| Del | Antall |
|---|---|
| ESP32-C3 SuperMini | 1 |
| Momentan trykknapp, 7 mm hull (PBS-110-typen) | 4 |
| LED på 10 mm (gjerne fire farger) | 4 |
| Motstand på 220 Ω (bruk 68–100 Ω til blå eller hvite LED-er) | 4 |
| Lite stykke stripboard, tynn ledning, krympestrømpe | |
| USB-C-kabel og en vanlig USB-lader | 1 |

## 2. Kobling

```
            ESP32-C3 SuperMini
           ┌──────────────────┐
Knapp 1 ───┤ GPIO 0    GPIO 5 ├─── 220Ω ──▶|── GND   LED 1
Knapp 2 ───┤ GPIO 1    GPIO 6 ├─── 220Ω ──▶|── GND   LED 2
Knapp 3 ───┤ GPIO 3    GPIO 7 ├─── 220Ω ──▶|── GND   LED 3
Knapp 4 ───┤ GPIO 4   GPIO 10 ├─── 220Ω ──▶|── GND   LED 4
           │ GND              │
           └──────────────────┘
Andre bein på alle knapper ── GND
```

- Hver knapp kobles mellom sin GPIO og GND. Det er ingen ekstra motstand, fordi ESP-en har intern pull-up.
- LED: det **lange** beinet (+) kobles mot motstanden, og det korte (−) til GND.
- Ikke bruk GPIO 2, 8 eller 9. Vil du bruke andre pinner, endrer du `firmware/Registreringsbryter/innstillinger.h`.

## 3. GitHub: data-repo og token

1. Lag et **privat** repo, for eksempel `registreringsbryter-data`. Kryss av for «Add a README».
2. Last opp `data-repo-mal/config.json` til repoet, og gjerne `data-repo-mal/README.md` også. Du kan også hoppe over dette og sette opp knappene fra nettsiden senere.
3. Lag en token: **GitHub → Settings → Developer settings → Personal access tokens → Fine-grained tokens → Generate new token**
   - *Repository access:* Only select repositories → velg data-repoet
   - *Permissions → Repository permissions → Contents:* **Read and write**
   - *Expiration:* så lenge som mulig. Når tokenet går ut, slutter boksen å laste opp. Hendelsene blir da liggende på boksen til du legger inn et nytt token.

   Bruk gjerne din personlige GitHub-konto. Organisasjoner krever ofte at fine-grained tokens godkjennes av en administrator.

## 4. Last opp firmware

### Arduino IDE (enklest)
1. Installer [Arduino IDE 2](https://www.arduino.cc/en/software).
2. **Boards Manager:** installer «esp32 by Espressif Systems» (versjon 3.x).
3. **Library Manager:** installer «WiFiManager» (av tzapu) og «ArduinoJson» (av Benoit Blanchon, 7.x).
4. Åpne `firmware/Registreringsbryter/Registreringsbryter.ino`.
5. **Tools → Board:** ESP32C3 Dev Module. **Tools → USB CDC On Boot:** Enabled.
6. Koble til USB-C, velg port og trykk **Upload**.
   - Hvis porten ikke vises: hold **BOOT**-knappen på kortet, trykk **RESET**, slipp BOOT og prøv igjen.

### PlatformIO (alternativ)
```powershell
cd firmware
pio run -t upload
pio device monitor
```

## 5. Første oppstart

1. Sett boksen i strøm. LED-ene lyser én etter én, og deretter går de i løpelys. Det betyr at boksen er i **oppsettmodus**.
2. Koble mobilen til WiFi-nettet **«Registreringsbryter»**. Oppsettsiden åpner seg, eller du kan gå til `192.168.4.1`.
3. Trykk **Configure WiFi**, velg nettverk og passord, og fyll inn:
   - **GitHub-bruker:** f.eks. `ola-nordmann`
   - **Data-repo:** f.eks. `registreringsbryter-data`. Hele lenken fungerer også.
   - **Gren:** `main`
   - **Token:** tokenet fra steg 3
4. Trykk **Save**. Boksen kobler seg til, og løpelyset stopper.

> WiFi som krever brukernavn og passord (WPA2-Enterprise, typisk på jobbnett) støttes ikke. Bruk gjestenett eller et nett med vanlig passord.

## 6. Daglig bruk

| Du ser | Betydning |
|---|---|
| LED lyser | Knappen er på, og tiden går |
| Løpelys på LED-ene som er av | Oppsettmodus. Koble til «Registreringsbryter» med mobilen. |
| Kort blink hvert 5. sekund | Boksen har ikke fått lastet opp på over 10 minutter. Sjekk WiFi eller token. Trykkene lagres og sendes senere. |

- **Strømbrudd:** Knapper som var på blir stoppet ved siste livstegn, som lagres hvert 2. minutt. Etter oppstart er alle knappene av.
- **Auto-av:** Alle knapper skrus av på klokkeslettet i `autoAv`, f.eks. 20:00. Det er nyttig når du glemmer å skru av før du går hjem.
- **BOOT-knappen** på kortet, hvis du når den:
  - Hold 3 s (alle LED-er lyser) og slipp: oppsettmodus, f.eks. for å bytte WiFi eller token.
  - Hold 10 s (LED-ene blinker raskt) og slipp: sletter alle innstillinger.

## 7. Nettsiden

`web/index.html` er én fil uten avhengigheter.

- **Lokalt:** dobbeltklikk på fila. Den åpnes i nettleseren og fungerer derfra.
- **GitHub Pages:** legg fila i et repo og skru på Pages. Pages fra et privat repo krever et betalt GitHub-abonnement. Siden inneholder ingen data, så den kan gjerne ligge i et offentlig repo. Dataene hentes fra det private repoet med tokenet ditt.

Første gang trykker du **⚙ Innstillinger** og legger inn bruker, repo og token. Det kan være samme token som boksen bruker. Tokenet lagres bare i nettleseren din.

Fra nettsiden kan du:
- se hvilke knapper som er på nå
- se tid per prosjekt for en dag, uke, måned eller et år
- se klokketid, altså hvor lenge minst én knapp var på
- se en liste over økter, og laste den ned som CSV (åpnes i Excel)
- bytte prosjekt og farge på knappene, med dato for når byttet gjelder fra
- sette auto-av

## Feilsøking

Åpne Serial Monitor (115200 baud) med boksen koblet til PC-en. Alt som skjer logges der.

| Problem | Løsning |
|---|---|
| Kobler ikke til WiFi | Mange SuperMini-kort har svak antenne. Sendeeffekten er allerede senket i koden. Prøv nærmere ruteren, og ikke legg metall rundt kortet. |
| `HTTP 401` | Tokenet er feil eller utløpt. Hold BOOT i 3 s og legg inn nytt. |
| `HTTP 403` / `404` | Tokenet har ikke tilgang til repoet, eller mangler «Contents: Read and write». |
| Feil klokkeslett | Boksen bruker norsk tid (`innstillinger.h`). Klokka hentes fra internett ved oppstart. |
