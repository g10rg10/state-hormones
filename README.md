# EMO Eyes — animazioni di comparsa (480×480)

Tool di anteprima per scegliere l'animazione con cui due occhi in stile **EMO**
(occhi a rettangolo arrotondato ciano, glow, su schermo nero) **compaiono** su uno
schermo **480×480 px**. Tutto **HTML + CSS + JS puro**, zero dipendenze, leggerissimo —
pensato per girare anche su display embedded.

## Avvio

```bash
python3 -m http.server 8123
# poi apri http://localhost:8123
```

## Cosa c'è nella gallery

- **Featured** in alto a dimensione reale **480×480**, in loop.
- **Griglia** con tutte le 9 varianti, ognuna in loop. Click per portarla in featured.
- Controlli:
  - **Velocità comparsa** — slider 150–800 ms (la durata del solo movimento di comparsa).
  - **Quadrato / Tondo** — schermo a rettangolo arrotondato o circolare.
  - **Blink** — micro-blink "idle" a metà ciclo per dare vita.
  - **Replay** — risincronizza tutte le animazioni.

## Le 9 varianti

| id | effetto |
|----|---------|
| `pop` | scatto dal piccolo con rimbalzo elastico |
| `eye-open` | apertura da fessura orizzontale (risveglio) |
| `fade-scale` | dissolvenza + leggero ingrandimento (la più calma) |
| `slide-in` | i due occhi entrano scivolando dai bordi |
| `power-on` | accensione LED/CRT: puntino bianco → flash → pieno |
| `drop-bounce` | caduta dall'alto con rimbalzo |
| `iris-expand` | apertura tipo iride: prima largo-piatto poi in altezza |
| `zoom-rush` | sfreccia da lontano, sfocato → a fuoco |
| `wipe-up` | rivelazione dal basso verso l'alto (clip-path) |

## Struttura / come estrarre uno stato per il firmware

- `index.html` — pagina della gallery.
- `eyes.css` — look dello schermo + occhi + blink + UI (la **base**).
- `keyframes.css` — le 9 animazioni: per ognuna un `@keyframes ap-<id>` + selettore `.anim-<id> .eye`.
- `anims.js` — elenco varianti (id, nome, descrizione, ms).
- `eyes.js` — controller dell'anteprima.

### Contratto delle animazioni

Tutte rispettano lo stesso contratto, così si possono comporre e scambiare:

```
DOM:   .stage > .eyes > .eye.left + .eye.right
ciclo: 0% nascosto → ~16% pienamente visibile (transform:none) →
       16–84% hold → 84–92% fade out → 92–100% nascosto
props: solo transform / opacity / filter (clip-path solo per i "wipe")
```

Per usarne **una sola** sul device basta: la base degli occhi da `eyes.css`, il blocco
della variante scelta da `keyframes.css`, e impostare la classe `anim-<id>` sullo `.stage`.
La velocità si controlla con la variabile CSS `--loop` (durata comparsa ≈ `--loop` × 0.16).

> Nota: gli **stati** (neutro, felice, sospetto, sonno, ecc.) e cosa devono fare
> verranno definiti dopo — questo è il primo giro sulle sole **comparse**, per tarare lo stile e la velocità.
