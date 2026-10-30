# Issue 001 — Aperfeiçoamento do overlay visual de OCR

**Status:** migrada para o GitHub — ver issues do repo
**Origem:** validação do mockup `mockup/` em 2026-10-29
**Screenshots de referência:**
- [estado inicial](assets/001-estado-inicial.png)
- [estado pós-OCR](assets/001-estado-pos-ocr.png)

## Rastreamento no GitHub

Resolvidas na v1 (fechadas):

- [#1 — Texto re-renderizado sobre o scan gera blur e desalinhamento](https://github.com/gustavo07henri/imv-ocr/issues/1)
  → solução: camada de texto invisível (padrão OCR de PDF)
- [#2 — Seleção com granularidade de caractere (caret-based)](https://github.com/gustavo07henri/imv-ocr/issues/2)
- [#3 — Seleção contínua entre linhas por arrasto](https://github.com/gustavo07henri/imv-ocr/issues/3)
- [#4 — Mapeamento de coordenadas das caixas OCR (zoom/pan)](https://github.com/gustavo07henri/imv-ocr/issues/4)

Pendentes (serão abertas como issues próprias quando priorizadas):

- Caixas e seleção com rotação ≠ 0 e/ou mirror
- Texto vertical CJK (`jpn_vert`, `chi_sim_vert`)
- Extensão palavra-a-palavra ao arrastar após duplo clique
- Intensidade do escurecimento/highlight configurável (`ocr_dim`, `ocr_highlight`)
- Cantos arredondados + padding nas caixas
- Animação de varredura (scanline) durante o processamento
- Hint visual de selecionabilidade (cursor `text`)
- OCR assíncrono (thread) — hoje a UI congela brevemente
- Clipboard nativo Wayland/X11 em vez de subprocesso `wl-copy`/`xclip`
- Comportamento em GIFs animados (frame atual) e SVG (rasterizar antes)

## Notas históricas (mockup)

- Mockup validado em Chromium headless; `backdrop-filter: brightness()`
  é o mecanismo do re-clareamento no CSS. No imv o efeito equivalente foi
  implementado com fill even-odd no cairo (escurecer tudo exceto as caixas).
