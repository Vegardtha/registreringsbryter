# CAD-spesifikasjon: kabinett for Registreringsbryter

> **Til Claude:** Denne fila beskriver boksen du skal modellere. Lag en **parametrisk OpenSCAD-modell** i `kabinett/kabinett.scad` som følger spesifikasjonen. Eksporter STL-filer og sjekk modellen mot sjekklista nederst. Alle mål er i millimeter. Mål merket **(mål selv)** er typiske verdier. Bruk dem som standard i parameterblokken, og be brukeren bekrefte dem hvis det er tvil.

---

## 1. Hva boksen er

En liten bordboks med **fire trykknapper** på rad. Over hver knapp sitter en **10 mm LED**. Inni ligger en **ESP32-C3 SuperMini**, og strømmen kommer via USB-C bak. Brukeren trykker på knappene mange ganger om dagen, så boksen må stå stødig og ikke gli.

## 2. Hovedform

- **Kile med skrå topp.** Fronten er lav og baksiden høy, så panelet heller mot brukeren.
- **To deler:**
  1. **`kasse`:** bunn og fire vegger, åpen oppe. Printes med bunnen ned, uten støtte.
  2. **`lokk`:** det skrå frontpanelet med hull for knapper og LED-er. Printes med utsiden ned mot printplata, så overflaten blir pen og hullene runde.
- Lokket skrues fast med **4 × M3-skruer** i hjørnestolper i kassa. Bruk hull på Ø 2,6 mm for selvgjengende skruer, eller Ø 4,0 mm for innpressede gjengeinnsatser (parameter).
- Ytre hjørner avrundes med radius 4 mm.

### Standardmål (parametre)

| Parameter | Standard | Kommentar |
|---|---|---|
| `bredde` | 110 | Ytre bredde (venstre–høyre) |
| `dybde` | 70 | Ytre dybde (front–bak) |
| `hoyde_front` | 32 | Ytre høyde foran |
| `hoyde_bak` | 46 | Ytre høyde bak. Gir ca. 11° helning. |
| `vegg` | 2.4 | Veggtykkelse for kassa |
| `bunn_tykkelse` | 2.4 | |
| `lokk_tykkelse` | 2.4 | **Maks 3,0** der knappene sitter, ellers rekker ikke mutteren gjengene |
| `hjorneradius` | 4 | |
| `toleranse` | 0.25 | Klaring for passform og hull |

## 3. Komponenter

### 3.1 Trykknapp (PBS-110-type, momentan) ×4
| Mål | Verdi |
|---|---|
| Gjenge | M7 → **hull Ø 7,2** |
| Mutter | 10 mm over flatene, ca. 2 mm tykk **(mål selv)** |
| Hus under panelet | Ø 9–10, lengde ca. 13 + 5 mm loddebein = **ca. 20 mm** fri høyde under panelet **(mål selv)** |
| Knapphette over panelet | Ø 6–7, ca. 9 mm høy |
| Gjengelengde | ca. 7 mm. Panelet må være ≤ 3 mm, fordi mutteren også skal ha plass. |

- Lag en **flat, sirkulær forsenkning på Ø 12 × 0,6 mm** på innsiden rundt hvert hull, slik at mutteren ligger plant.
- Fri høyde: under hver knapp må det være **minst 22 mm** fra undersiden av lokket til innsiden av bunnen, målt loddrett. Kontroller dette for den fremste knappen, der boksen er lavest.

### 3.2 LED 10 mm ×4
| Mål | Verdi |
|---|---|
| Kropp | Ø 10,0 |
| Krage | **Ø 11,0** (bredeste punkt), ca. 1,5 mm tykk **(mål selv)** |
| Hull | **Ø 10,2** (presspasning, LED-en trykkes inn fra innsiden) |
| Lengde under panelet | ca. 14 mm + bein **(mål selv)** |

- LED-en trykkes inn fra innsiden. Kroppen stikker opp gjennom panelet, og kragen på Ø 11 stopper mot innsiden.
- Lag en **Ø 11,6 × 1,0 mm forsenkning på innsiden** rundt LED-hullet, slik at kragen ligger plant og LED-en stikker litt lenger ut.
- Parametre: `led_hull = 10.2`, `led_krage = 11.0`, `led_krage_klaring = 0.6`. LED-en skal sitte stramt. Er hullet for trangt, øker brukeren `led_hull` med 0,1 mm og printer en ny test.
- Valgfritt: en **testbit** (`del = "led_test"`), en liten plate på 30 × 15 × 2,4 mm med tre hull på Ø 10,0, 10,2 og 10,4, så brukeren kan finne riktig passform før hele lokket printes.

### 3.3 ESP32-C3 SuperMini ×1
| Mål | Verdi |
|---|---|
| Kretskort | **22,5 × 18,0**, tykkelse 1,0–1,6 **(mål selv)** |
| Komponenter | Bare på oversiden, maks ca. 3,5 mm høye |
| USB-C | Midt på den ene kortsiden, på oversiden. Kontakten er ca. 9,0 × 3,2 og stikker ca. 1 mm utenfor kortkanten. |
| Antenne | Ved kortsiden motsatt USB-C. **Ikke legg den nær metall** (knappehus). |
| Stiftlister | Loddes **ikke** på. Ledningene loddes rett i hullene. |

- Kortet ligger flatt på bunnen i en **holder**: to skinner langs langsidene med spor til kortkanten (klaring 0,3), en endestopp foran og en avstand på 1 mm fra bunnen.
- **USB-C peker ut gjennom bakveggen.** Kortkanten ligger helt inntil innsiden av bakveggen.
- **Hull i bakveggen for USB-C:** et avrundet rektangel på **13,0 × 7,5**, sentrert på kontakten. Det må være stort nok til at støpselet på kabelen går helt inn. Senter av hullet ligger i høyde med kontakten (bunn + avstand + kortets tykkelse + 1,6).
- Plasser holderen så **antennen er minst 10 mm unna knappenes metallhus**. Sett den gjerne bak til venstre eller høyre, utenfor knapperaden.

### 3.4 Stripboard (valgfritt)
- Et flatt område på ca. **35 × 25** på bunnen ved siden av ESP-en, med to tapper (Ø 5 × 4 mm høye, hull Ø 1,8 for M2-skruer) eller et enkelt spor. Bruk parameteren `med_stripboard = true`.

## 4. Oppsett av frontpanelet (lokket)

Sett fra brukerens side, der fronten er nærmest:

```
  bak (høy)
 ┌───────────────────────────────────────────┐
 │     ●          ●          ●          ●     │  ← LED-er (Ø 10,2)
 │                                            │
 │    (◯)        (◯)        (◯)        (◯)    │  ← knapper (Ø 7,2)
 │    [  1 ]     [  2 ]     [  3 ]     [  4 ] │  ← etikettfelt + nummer
 └───────────────────────────────────────────┘
  front (lav)
```

| Parameter | Standard |
|---|---|
| `knapp_avstand` | 24 (senter til senter) |
| Knapperaden | Sentrert i bredden, ca. 55 % av panelets lengde fra fronten |
| `led_avstand` | 17 bakover fra senter av hver knapp. Det gir ca. 5 mm mellom mutteren (Ø 11) og LED-kragen (Ø 11,6). |
| Etikettfelt | 20 × 9, forsenket 0,4 mm, 9 mm foran senter av hver knapp. Her limer brukeren en lapp med prosjektnavnet. |
| Nummer | «1» til «4», gravert 0,6 mm i nedre høyre hjørne av hvert etikettfelt, skrift 4 mm |

- Ingen hull eller skruestolper må komme i konflikt med knappenes hus, muttere eller LED-er. Ta med klaring for mutteren (Ø 11) og LED-kragen (Ø 11,6).
- Med `knapp_avstand` 24 og LED-krage på Ø 11,6 er det ca. 12 mm mellom LED-ene. Det er nok.
- Lokket ligger **innfelt** i en 1 mm kant (falsen) øverst i kassa, eller med en innvendig leppe på 1,5 mm som griper ned i kassa. Det hindrer at lokket forskyver seg.

## 5. Kassa

- 4 **hjørnestolper**, Ø 7, med skruehull. Toppen følger skråningen, så lokket hviler på alle fire.
- **USB-C-hull** i bakveggen (se 3.3).
- **4 forsenkninger for gummiføtter** under bunnen: Ø 10,5 × 1,0, 8 mm fra hjørnene.
- Valgfritt **vektlomme** i bunnen: 40 × 30 × 6 mm, til skiver eller mynter som gjør boksen tyngre. Parameter `med_vektlomme = true`.
- Ingen ventilasjon er nødvendig.

## 6. Leveranser

1. `kabinett/kabinett.scad` med
   - en parameterblokk øverst med alle mål fra denne fila og egne kommentarer for de som er **(mål selv)**
   - modulene `kasse()`, `lokk()`, `samlet()` (lokket på plass og enkle «dummy»-komponenter for kontroll) og `print_plate()` (begge delene lagt ut riktig vei for printing)
   - en variabel `del = "samlet"; // "kasse" | "lokk" | "samlet" | "print"`
2. STL-filer: `kabinett/kasse.stl` og `kabinett/lokk.stl`
   ```powershell
   openscad -o kabinett/kasse.stl -D 'del="kasse"' kabinett/kabinett.scad
   openscad -o kabinett/lokk.stl  -D 'del="lokk"'  kabinett/kabinett.scad
   ```
3. Et forhåndsvisningsbilde av `samlet` som PNG, `kabinett/forhandsvisning.png`:
   ```powershell
   openscad -o kabinett/forhandsvisning.png -D 'del="samlet"' --imgsize=1200,900 --viewall --autocenter kabinett/kabinett.scad
   ```
   Se på bildet og rett feil før du sier deg ferdig.

## 7. Printinnstillinger (skriv dem inn i kommentar øverst i .scad-fila)

- PLA eller PETG, 0,2 mm lag, 3 perimetre, 15 % fyll, **ingen støtte**
- `kasse`: bunnen ned. `lokk`: utsiden ned.
- Print gjerne bare lokket først som test, for å sjekke at knapper og LED-er passer.

## 8. Sjekkliste før du er ferdig

- [ ] Det er **≥ 22 mm** fri høyde under hver knapp, også den fremste.
- [ ] Lokket er **≤ 3,0 mm** tykt der knappene sitter.
- [ ] Det er ingen kollisjon mellom knapper, muttere (Ø 11), LED-er (krage Ø 11,6, ca. 14 mm lange under panelet), skruestolper og ESP-holderen. Kontroller med dummy-komponenter i `samlet()`.
- [ ] USB-C-hullet ligger midt på kontakten, og kortkanten ligger inntil veggen.
- [ ] Antennen på ESP-en er ≥ 10 mm fra knappehusene.
- [ ] Ingen overheng over 45° i printretningen, så det trengs ingen støtte.
- [ ] Begge STL-filene er «manifold». OpenSCAD skal ikke gi CGAL-advarsler.
- [ ] Alle mål merket **(mål selv)** ligger samlet i parameterblokken.
