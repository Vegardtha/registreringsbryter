# CAD-spesifikasjon: kabinett for Registreringsbryter

> **Til Claude:** Denne fila beskriver boksen du skal modellere. Lag en **parametrisk OpenSCAD-modell** i `kabinett/kabinett.scad` som følger spesifikasjonen. Eksporter STL-filer og sjekk modellen mot sjekklista nederst. Alle mål er i millimeter. Mål merket **(mål selv)** er typiske verdier. Bruk dem som standard i parameterblokken, og be brukeren bekrefte dem hvis det er tvil.

---

## 1. Hva boksen er

En liten bordboks med **fire store knapper** på rad. Hver knapp er en **3D-printet hette på Ø 24 mm** som sitter over en liten 7 mm trykknapp. Over hver knapp sitter en **10 mm LED**. Inni ligger en **ESP32-C3 SuperMini**, og strømmen kommer via USB-C bak. Brukeren trykker på knappene mange ganger om dagen, så boksen må stå stødig og ikke gli, og knappene må være store og lette å treffe.

## 2. Hovedform

- **Kile med skrå topp.** Fronten er lav og baksiden høy, så panelet heller mot brukeren.
- **Tre slags deler:**
  1. **`kasse`:** bunn og fire vegger, åpen oppe. Printes med bunnen ned, uten støtte.
  2. **`lokk`:** det skrå frontpanelet med hull for knapper og LED-er, og føringsringer for hettene. Printes med **toppen opp**, slik at føringsringene står opp uten overheng. Undersiden er flat bortsett fra forsenkningene for LED-kragene. Bruk gjerne «ironing» for pen toppflate.
  3. **`hette`** ×4: de store knappene. Printes med toppen ned.
- Lokket skrues fast med **4 × M3-skruer** i hjørnestolper i kassa. Bruk hull på Ø 2,6 mm for selvgjengende skruer, eller Ø 4,0 mm for innpressede gjengeinnsatser (parameter).
- Ytre hjørner avrundes med radius 4 mm.

### Standardmål (parametre)

| Parameter | Standard | Kommentar |
|---|---|---|
| `bredde` | 140 | Ytre bredde (venstre–høyre) |
| `dybde` | 80 | Ytre dybde (front–bak) |
| `hoyde_front` | 32 | Ytre høyde foran |
| `hoyde_bak` | 48 | Ytre høyde bak. Gir ca. 11° helning. |
| `vegg` | 2.4 | Veggtykkelse for kassa |
| `bunn_tykkelse` | 2.4 | |
| `lokk_tykkelse` | 2.4 | **Maks 3,0** der knappene sitter, ellers rekker ikke mutteren gjengene |
| `hjorneradius` | 4 | |
| `toleranse` | 0.25 | Klaring for passform og hull |

## 3. Komponenter

### 3.1 Trykknapp (PBS-110-type, momentan) ×4
| Mål | Verdi |
|---|---|
| Gjenge | M7 → **hull Ø 7,2** (testet: passer) |
| Montering | Huset sitter **under** panelet. Gjengen går opp gjennom hullet, og **mutteren skrus på fra oversiden**. |
| Mutter | 10 mm over flatene (ca. Ø 11,5 over hjørnene), ca. 2 mm tykk **(mål selv)** |
| Hus under panelet | Ø 9–10, lengde ca. 13 + 5 mm loddebein = **ca. 20 mm** fri høyde under panelet **(mål selv)** |
| Original knapphette (blå) | **Ø 5,8–5,9** (målt). Toppen står ca. **9 mm over panelet** **(mål selv)**. |
| Vandring | Ca. **2 mm** fra hvile til klikk **(mål selv)** |
| Gjengelengde | ca. 7 mm. Panelet må være ≤ 3 mm, fordi mutteren også skal ha plass. |

- Undersiden av panelet er flat rundt hvert hull, så skulderen på knappehuset ligger an.
- Fri høyde: under hver knapp må det være **minst 22 mm** fra undersiden av lokket til innsiden av bunnen, målt loddrett. Kontroller dette for den fremste knappen, der boksen er lavest.

### 3.2 Stor knapphette (printet) ×4

Hetten tres ned over den blå knapphetten og sitter fast med presspasning. En føringsring på panelet hindrer at den vipper.

```
        ┌──────────────────────┐   ← topp, svakt innsunket (Ø 24)
        │  ┌────────────────┐  │
        │  │    ┌──────┐    │  │   ← hylse som tres over den blå hetten
   ┌──┐ │  │    │      │    │  │ ┌──┐ ← føringsring (fast i lokket)
   │  │ │  │    └──────┘    │  │ │  │
   │  │ └──┘  (blå hette)   └──┘ │  │ ← skjørt
═══╧══╧══════════[mutter]════════╧══╧═══ panel
```

| Parameter | Standard | Kommentar |
|---|---|---|
| `hette_diameter` | 24 | Ytre diameter. Kan økes; da må `knapp_avstand` økes like mye. |
| `hette_topp` | 2.4 | Tykkelse på toppen |
| `hette_grop` | 0 | Sfærisk fingergrop i toppen. Står på 0 (flat), fordi hetten printes med toppen ned og en grop da ville henge i lufta. |
| `hette_vegg` | 1.6 | Skjørtets veggtykkelse |
| `hylse_indre` | 6.2 | Testet mot 6,0 og 6,1: 6,2 mm sitter perfekt. Hylsa er `blaa_hette_d + hylse_tillegg` = 5,9 + 0,3. |
| `hylse_vegg` | 1.6 | |
| `hylse_dybde` | 4.0 | Hvor langt hylsa tres ned over den blå hetten |
| `vandring` | 2.0 | **(mål selv)** |
| `foring_hoyde` | 6.5 | Føringsringens høyde over panelet |
| `foring_vegg` | 1.6 | |
| `foring_klaring` | 0.3 | Radiell klaring mellom skjørt og føringsring |

**Krav som modellen må oppfylle, og som skal regnes ut i koden:**
- Når hetten er i hvile, sitter undersiden av toppen rett på den blå hetten. Toppen av hetten står da på `blaa_hette_hoyde + hette_topp` over panelet.
- **Skjørtet:** i hvile skal det være `vandring + 1,0` mm fra skjørtets underkant ned til panelet, så hetten ikke bunner ut før knappen klikker.
- **Hylsa:** i hvile skal det være minst `vandring + 0,5` mm fra hylsas underkant ned til toppen av mutteren.
- **Overlapp:** i hvile skal skjørtet og føringsringen overlappe minst **3 mm** i høyden, så hetten ikke vipper.
- Skjørtets innside må gå klar av mutteren (Ø 11,5 over hjørnene) med god margin.
- Hetten printes med toppen ned. Hylsa og skjørtet står da rett opp, og det trengs ingen støtte. Ytterkanten på toppen får en fas på 0,6 mm.
- Lag en **testbit** (`del = "hette_test"`): en flat plate på 40 × 40 × 2,4 mm med én føringsring og ett knappehull. Da kan brukeren sjekke passform, vandring og klikk med én hette før hele lokket printes.

### 3.3 LED 10 mm ×4
| Mål | Verdi |
|---|---|
| Kropp | Ø 10,0 |
| Krage | **Ø 11,0** (bredeste punkt), ca. 1,5 mm tykk **(mål selv)** |
| Hull | **Ø 9,9** (testet: passer best). Ribber kan skrus på med `led_ribbe_d`. |
| Høyde | ca. 13,8 mm fra underkant krage til toppen av kuppelen **(mål selv)** |

- LED-en trykkes inn fra innsiden. Kroppen stikker opp gjennom panelet, og kragen på Ø 11 stopper mot innsiden. Over panelet stikker LED-en ca. 11 mm opp. Under panelet er det bare beina.
- Lag en **Ø 11,6 × 1,0 mm forsenkning på innsiden** rundt LED-hullet, slik at kragen ligger plant og LED-en stikker litt lenger ut.
- **Kragen har to motstående flate sider** og er rund ellers. Den er Ø 11,0 på det bredeste og 10,0 mellom de flate sidene. Forsenkningen har samme form (`led_krage_flat`), så kragen ligger plant og LED-en ikke kan vris. De flate sidene peker mot venstre og høyre (`led_flat_retning = 0`).
- Parametre: `led_hull = 9.9`, `led_ribbe_d = 0`, `led_ribber = 6`, `led_krage = 11.0`, `led_krage_flat = 10.0`, `led_krage_klaring = 0.4`. Sett `led_ribbe_d` til f.eks. 9,8 og `led_hull` til 10,4 for et hull med ribber.
- **Testbit** (`del = "led_test"`): en plate med tre runde hull på 9,8, 9,9 og 10,0 mm, alle med forsenkning for kragen på undersiden. Tidligere tester viste at 9,9 passer best.

### 3.4 ESP32-C3 SuperMini ×1
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

### 3.5 Stripboard (valgfritt)
- Et flatt område på ca. **35 × 25** på bunnen ved siden av ESP-en, med to tapper (Ø 5 × 4 mm høye, hull Ø 1,8 for M2-skruer) eller et enkelt spor. Bruk parameteren `med_stripboard = true`.

## 4. Oppsett av frontpanelet (lokket)

Sett fra brukerens side, der fronten er nærmest:

```
  bak (høy)
 ┌────────────────────────────────────────────────────┐
 │      ●             ●             ●             ●     │  ← LED-er (Ø 9,9)
 │                                                    │
 │    ╭───╮         ╭───╮         ╭───╮         ╭───╮   │
 │    │ 1 │         │ 2 │         │ 3 │         │ 4 │   │  ← store hetter (Ø 24) i føringsringer
 │    ╰───╯         ╰───╯         ╰───╯         ╰───╯   │
 │   [      ]      [      ]      [      ]      [      ]  │  ← etikettfelt + nummer
 └────────────────────────────────────────────────────┘
  front (lav)
```

Alle avstander er målt langs panelet fra frontkanten, til senter av elementet.

| Parameter | Standard | Kommentar |
|---|---|---|
| `knapp_avstand` | 32 | Senter til senter. Gir ca. 4 mm mellom føringsringene. |
| `etikett_pos` | 12 | Etikettfelt: 26 × 9, forsenket 0,4 mm. Her limer brukeren en lapp med prosjektnavnet. |
| `knapp_pos` | 33 | Senter av knapp, hette og føringsring |
| `led_pos` | 56 | Senter av LED. Det gir ca. 3 mm fra føringsringen til LED-forsenkningen. |
| Nummer | | «1» til «4», gravert 0,6 mm i nedre høyre hjørne av hvert etikettfelt, skrift 4 mm |

- Knapperaden er sentrert i bredden.
- **Føringsringene** er en del av lokket. Innvendig diameter er `hette_diameter + 2 × foring_klaring`.
- Ingen hull eller skruestolper må komme i konflikt med føringsringer, knappenes hus eller LED-er. På undersiden må det også være plass til LED-kragen (Ø 11,6).
- Lokket ligger oppå veggene og dekker hele fotavtrykket. De fire skruene holder det på plass. Det er ingen leppe på undersiden, fordi lokket da ikke kunne printes flatt med toppen opp.

## 5. Kassa

- 4 **hjørnestolper**, Ø 7, med skruehull. Toppen følger skråningen, så lokket hviler på alle fire.
- **USB-C-hull** i bakveggen (se 3.4).
- **4 forsenkninger for gummiføtter** under bunnen: Ø 10,5 × 1,0, 8 mm fra hjørnene.
- Valgfritt **vektlomme** i bunnen: 40 × 30 × 6 mm, til skiver eller mynter som gjør boksen tyngre. Parameter `med_vektlomme = true`.
- Ingen ventilasjon er nødvendig.

## 6. Leveranser

1. `kabinett/kabinett.scad` med
   - en parameterblokk øverst med alle mål fra denne fila og egne kommentarer for de som er **(mål selv)**
   - modulene `kasse()`, `lokk()`, `hette()`, `samlet()` (lokket på plass, hetter i hvile og enkle «dummy»-komponenter for kontroll) og `print_plate()` (alle delene lagt ut riktig vei for printing, med 4 hetter)
   - en variabel `del = "samlet"; // "kasse" | "lokk" | "hette" | "samlet" | "print" | "hette_test" | "led_test"`
   - `echo()`-linjer som skriver ut de utregnede klaringene fra 3.2 (skjørt–panel, hylse–mutter, overlapp) og fri høyde under hver knapp
2. STL-filer: `kabinett/kasse.stl`, `kabinett/lokk.stl`, `kabinett/hette.stl` (én hette, printes 4 ganger) og `kabinett/hette_test.stl`
   ```powershell
   openscad -o kabinett/kasse.stl      -D 'del="kasse"'      kabinett/kabinett.scad
   openscad -o kabinett/lokk.stl       -D 'del="lokk"'       kabinett/kabinett.scad
   openscad -o kabinett/hette.stl      -D 'del="hette"'      kabinett/kabinett.scad
   openscad -o kabinett/hette_test.stl -D 'del="hette_test"' kabinett/kabinett.scad
   ```
3. Et forhåndsvisningsbilde av `samlet` som PNG, `kabinett/forhandsvisning.png`:
   ```powershell
   openscad -o kabinett/forhandsvisning.png -D 'del="samlet"' --imgsize=1200,900 --viewall --autocenter kabinett/kabinett.scad
   ```
   Se på bildet og rett feil før du sier deg ferdig.

## 7. Printinnstillinger (skriv dem inn i kommentar øverst i .scad-fila)

- **PLA** for alle delene: 0,2 mm lag, 3 perimetre, 15 % fyll, **ingen støtte**
- Ca. 210 °C dyse, 60 °C plate og full kjølevifte (vanlige PLA-innstillinger)
- PLA er stivt og gir lite etter, så presspasningene (`hylse_tillegg`, `led_hull`) bør testes med testbitene før alt printes
- Ikke la boksen stå i direkte sol eller i en varm bil. PLA blir mykt rundt 55 °C.
- `kasse`: bunnen ned. `lokk`: toppen opp. `hette`: toppen ned.
- Hettene kan gjerne printes i samme farge som LED-en over dem.
- **Rekkefølge:** print `hette_test` og en hette først. Når passform og klikk er riktig, printer du lokket og så kassa.

## 8. Sjekkliste før du er ferdig

- [ ] Det er **≥ 22 mm** fri høyde under hver knapp, også den fremste.
- [ ] Lokket er **≤ 3,0 mm** tykt der knappene sitter.
- [ ] Klaringene for hetten fra 3.2 er oppfylt (skjørt–panel ≥ vandring + 1, hylse–mutter ≥ vandring + 0,5, overlapp ≥ 3), og verdiene skrives ut med `echo()`.
- [ ] Det er ingen kollisjon mellom føringsringer, hetter, knapper, muttere, LED-er (krage Ø 11,6), skruestolper og ESP-holderen. Kontroller med dummy-komponenter i `samlet()`.
- [ ] USB-C-hullet ligger midt på kontakten, og kortkanten ligger inntil veggen.
- [ ] Antennen på ESP-en er ≥ 10 mm fra knappehusene.
- [ ] Ingen overheng over 45° i printretningen, så det trengs ingen støtte.
- [ ] Alle STL-filene er «manifold». OpenSCAD skal ikke gi CGAL-advarsler.
- [ ] Alle mål merket **(mål selv)** ligger samlet i parameterblokken.
