# Angel Protocol — Invariantes

## Tamaños
- PAYLOAD_MAX = 40
- FRAG_CHUNK = 20
- FRAG_MAX = 32
- CAP MENSAJE = FRAG_MAX * FRAG_CHUNK = 640

## Reglas
- proto_enviar: len <= PAYLOAD_MAX, dest NULL = broadcast (0xFF*4)
- frag_recibir: chunk <= FRAG_CHUNK, total <= FRAG_MAX, idx < total
- frag_enviar: len <= FRAG_MAX * FRAG_CHUNK
- crypto_encrypt: len <= PAYLOAD_MAX

## Bugs corregidos
- Bug 5: out[1280] no va a pila
- Bug 6: chunk > FRAG_CHUNK se rechaza
- Bug 6b: total_len > 640 se rechaza
- Bug 8b: dest NULL usa marca broadcast
