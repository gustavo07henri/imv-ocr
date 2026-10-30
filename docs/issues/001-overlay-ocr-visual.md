# Issue 001 — Aperfeiçoamento do overlay visual de OCR

**Status:** aberta (v1 validada via mockup, melhorias adiadas)
**Origem:** validação do mockup `mockup/` em 2026-10-29
**Screenshots de referência:**
- [estado inicial](assets/001-estado-inicial.png)
- [estado pós-OCR](assets/001-estado-pos-ocr.png)

## Decisões de design validadas (baseline v1)

- Texto do overlay **invisível**: o que se vê é o texto do próprio scan;
  o overlay existe apenas para seleção/cópia (padrão de camada OCR de PDF).
  Motivo: sobreposição de texto re-renderizado nunca fica 100% alinhada
  ao scan e gera borrões.
- Imagem escurecida (`brightness ~0.42`) com **re-clareamento apenas atrás
  das linhas de texto** detectadas (~×2.3), funcionando como "holofote".
- Botão de OCR com efeito vidro (fundo translúcido + blur), canto
  inferior direito, usando `icone_scanear_pagina.svg`.
- Seleção por arrasto com destaque azul translúcido; `Ctrl+C` copia o
  texto extraído.

## Melhorias futuras

### Seleção
- [ ] **Granularidade por palavra**: o mockup destaca linhas inteiras;
      com as bounding boxes por palavra do tesseract
      (`TessResultIterator` em nível `RIL_WORD`) seria possível selecionar
      trechos parciais de linha.
- [ ] Seleção contínua entre linhas (arrastar de uma linha para outra
      selecionando o intervalo, como em leitores de PDF).

### Visual
- [ ] Tornar intensidade do escurecimento/clareamento configurável
      (opções no `[options]` do `imv_config`, ex.: `ocr_dim = 0.42`,
      `ocr_highlight = 2.3`).
- [ ] Cantos arredondados e padding nas caixas de destaque (no imv será
      desenho manual via GL/cairo — o mockup usa `border-radius` de graça).
- [ ] Animação de varredura (scanline) durante o processamento —
      avaliar custo de implementar como overlay animado no loop de render.
- [ ] Hint visual de selecionabilidade: no mockup o hover contorna a
      linha; definir equivalente no imv (cursor `text` ao passar sobre
      uma região de OCR).

### Integração com o imv (débitos técnicos do mockup)
- [ ] **Mapeamento de coordenadas**: as bboxes do tesseract vêm em pixels
      da imagem; precisam ser transformadas pelo estado do viewport
      (zoom/pan/rotação) a cada redraw — o mockup só lida com resize.
- [ ] OCR assíncrono (thread) para não congelar a UI em imagens grandes.
- [ ] Integração com clipboard nativo (Wayland `wl_data_device` / X11
      selection) para o `Ctrl+C` — hoje imv não tem suporte a clipboard.
- [ ] Comportamento em GIFs animados (rodar no frame atual) e em
      imagens SVG (rasterizar antes).

## Notas

- Mockup validado em Chromium headless; `backdrop-filter: brightness()`
  é o mecanismo do re-clareamento no CSS. No imv o efeito equivalente
  será: renderizar a imagem com brilho reduzido e re-desenhar as regiões
  das bboxes com brilho normal + caixa translúcida clara por cima.
