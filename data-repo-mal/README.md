# Tidsdata fra Registreringsbryter

Dette repoet fylles automatisk av registreringsboksen. Ikke rediger filene i `data/` for hånd.

## Filer

| Fil | Hva |
|---|---|
| `config.json` | Hvilket prosjekt hver knapp hører til, og når boksen skal skru av alle knapper automatisk. Endres enklest fra nettsiden (⚙ Innstillinger). |
| `data/ÅÅÅÅ/MM/ÅÅÅÅ-MM-DD.jsonl` | Én fil per dag, én linje per trykk. |

## config.json

```json
{
  "autoAv": "20:00",
  "knapper": {
    "1": [
      { "fra": "2026-11-01", "prosjekt": "Gammelt prosjekt", "farge": "#e5484d" },
      { "fra": "2026-11-24", "prosjekt": "Nytt prosjekt",    "farge": "#30a46c" }
    ]
  }
}
```

- `autoAv`: klokkeslett (norsk tid) da alle knapper skrus av, eller `null` for aldri.
- `knapper`: en liste per knapp. Den siste oppføringen med `fra` på eller før en dato gjelder for den datoen. Slik blir gammel tid stående på riktig prosjekt når du bytter.

## Hendelser

```json
{"tid":"2026-11-24T08:14:03Z","knapp":2,"hendelse":"start","kilde":"knapp"}
{"tid":"2026-11-24T11:02:41Z","knapp":2,"hendelse":"stopp","kilde":"knapp"}
```

- `tid` er i UTC.
- `kilde` er `knapp` (trykk), `auto` (autoAv), `strombrudd` (boksen mistet strøm mens knappen var på; stopp er satt til siste livstegn) eller `nullstilling`.
