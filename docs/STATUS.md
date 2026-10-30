# Status do projeto — imv + OCR

**Última atualização:** 2026-10-30
**Branch:** `ocr-feature` (baseada na tag `v5.0.1`)
**Publicado em:** https://github.com/gustavo07henri/imv-ocr (público;
`origin` = GitHub, `upstream` = sr.ht original para futuros merges)

## O que foi feito

### Preparação
- Repositório clonado de `https://git.sr.ht/~exec64/imv` na tag **v5.0.1**,
  branch de trabalho `ocr-feature`.
- Dependências verificadas: tesseract 5.5.3, leptonica 1.87, meson, ninja OK.
  **`tesseract-data-por` ainda não instalado** (sudo pendente no terminal).

### Design (validado)
- Mockup em `mockup/` (HTML/CSS/JS) validado com screenshots headless:
  `docs/issues/assets/001-estado-inicial.png` e `001-estado-pos-ocr.png`.
- Issue de aperfeiçoamento futuro: `docs/issues/001-overlay-ocr-visual.md`.

### Instalação migrada para ~/.local (2026-10-30)
- Binário movido de `/usr/local` (root) para **`~/.local`** (usuário, sem
  sudo): `~/.local/bin/imv*` + `~/.local/share/applications/imv*.desktop`.
  Precedência no PATH e no XDG_DATA_DIRS mantém o shadowing do pacote da
  distro. E2E revalidado com o binário de `~/.local`.
- **`install-viewer.sh`** (novo): instala/reinstala idempotente em `~/.local`
  e remove restos de `/usr/local` (única parte com sudo). É o método de
  restauração após updates/reinstalação do sistema — embora updates do
  pacote `imv` não toquem `~/.local`.
- **`rollback-viewer.sh`** reescrito: remove `~/.local` e restos de
  `/usr/local`.
- README.md reescrito como README do fork (motivação, instalação, uso,
  stack, limitações; aviso de versão inicial testada só no Omarchy 4.0.4,
  créditos ao autor original, menção ao uso de IA).
- Nota no Obsidian: `600 tutoriais/imv com OCR — instalação, restauração e
  rollback.md`.
- Config do usuário (`~/.config/imv/config`) **não é tocada**: binds OCR são
  padrão compilado e coexistem com os binds pessoais.

### Instalação no sistema (2026-10-30)
- Build instalado em `/usr/local` (`sudo meson install -C build`), sombreando
  o pacote `imv 5.0.1-2` do pacman (que permanece intacto em `/usr`).
  `/usr/local/bin` precede `/usr/bin` no PATH e `/usr/local/share` precede
  `/usr/share` no `XDG_DATA_DIRS`, então o `imv.desktop` padrão já abre a
  versão OCR — nenhuma alteração em `mimeapps.list` foi necessária.
- **Rollback:** `./rollback-viewer.sh` (raiz do repo) — roda
  `sudo ninja uninstall` e o sistema volta ao imv da distro imediatamente.

### BUG RAIZ encontrado — eventos de botão repetidos no Wayland (2026-10-30)
- **Diagnóstico via logs do usuário:** durante UM arrasto físico, o handler
  recebia `press` repetidamente (click_count chegou a 237), intercalado com
  `drag` — a âncora da seleção era resetada a cada frame de movimento. Isso
  explica TODOS os relatos: v1 "só palavras individuais" (sel_start
  resetava para a palavra sob o cursor), v3 "comportamento inalterado".
- **Causa:** `src/wl_window.c`, `pointer_frame()` — após emitir
  `IMV_EVENT_MOUSE_BUTTON`, a flag `pointer.change.dmouse1` nunca era limpa
  (diferente de `dx/dy`/`scroll`, que são consumidos). Cada frame posterior
  re-emitia o evento com `pressed=true`. Bug **latente do imv original**:
  nenhum código consumia eventos de botão antes do OCR. Corrigido com
  `dmouse1 = false` após o push. X11 não é afetado (eventos discretos
  ButtonPress/Release).
- Verificado: build limpo, select_all+copia via IPC OK.
- **VALIDADO pelo usuário com mouse real (2026-10-30):** log mostra 1 press,
  drags com caret progressivo, 1 release com intervalo exato
  (`[1..836)`, `[1..56)`); duplo clique = palavra `[994..998)`, triplo =
  linha `[951..1091)`. Comportamento aprovado ("aceitável").
- O fix de `dmouse1` é um bug do imv upstream — candidato a patch para o
  projeto original (sr.ht/~exec64/imv).

### Reescrita da seleção — padrão editor (2026-10-30, terceiro ajuste)
- **Análise limpa do "comportamento inalterado":** binário instalado em
  `/usr/local` estava com md5 idêntico ao build (descarta binário velho);
  escala do monitor = 1 (descarta HiDPI); caminho de eventos Wayland
  auditado (`pointer_button` BTN_LEFT 0x110 → `mouse1` → `IMV_EVENT_MOUSE_BUTTON`
  button=1 → handler). Causa provável do relato: teste feito com a versão
  anterior (granularidade de palavra) e/ou impossibilidade de eu testar
  mouse real via IPC. Contramedida: **logs de diagnóstico** (`IMV_OCR_DEBUG=1`)
  em press/drag/release — se o evento não chegar, o log não aparece.
- **Reescrita do mapeamento ponto→caret no padrão editor:** primeiro escolhe
  a linha de texto mais próxima verticalmente, depois caminha os chars da
  linha até o primeiro centro além do cursor. Clicar na margem ao lado de
  uma linha agora ancora no início/fim dela (antes: caractere mais próximo
  em 2D, que podia pular de linha).
- **Contagem de cliques:** 1 = caret/limpa seleção, 2 = palavra, 3 = linha.
- **Escape em dois estágios:** primeira vez solta a seleção, segunda limpa
  o overlay (padrão "dismiss do estado mais interno").
- Verificado via IPC: select_all+copia OK, clear em 2 estágios OK.
- **Pendente:** teste manual com mouse real — protocolo:
  `IMV_OCR_DEBUG=1 imv <imagem>`, arrastar sobre o texto e conferir os logs
  `ocr: press ... / ocr: drag caret=... / ocr: release ...`.

### Seleção caractere a caractere (2026-10-30, segundo ajuste)
- **Diagnóstico:** seleção anterior era por índice de palavra inteira +
  hit-test só dentro das caixas → impossível selecionar parte de palavra,
  começar o arrasto na margem, ou alcançar o texto completo.
- **Novo modelo (padrão editor/PDF):** tesseract agora itera em `RIL_SYMBOL`
  gerando `chars[]` com bbox por caractere + flags `word_start`/`line_start`
  (via `TessPageIteratorIsAtBeginningOf` com `RIL_WORD`/`RIL_TEXTLINE` —
  fronteiras de linha vêm do tesseract, não de heurística geométrica, o que
  corrigiu quebras espúrias em vírgulas).
- Seleção = par de carets `sel_anchor`/`sel_focus` (0..char_count). Clique
  ancora no caret mais próximo (mesmo fora do texto); arrastar move o focus;
  **duplo clique seleciona a palavra**; novo comando `ocr_select_all`
  (bind `<Ctrl+a>`) + `ocr_copy` continua copiando só a seleção.
- Highlight: runs contínuos de chars por linha (azul translúcido), idêntico
  à seleção de texto comum.
- Cópia monta o texto char a char: `\n` em `line_start`, espaço em
  `word_start` (suprimido antes de pontuação `,.;:!?%)]}`).
- Corrigido leak: iteradores do tesseract agora são destruídos
  (`TessResultIteratorDelete`).
- Verificado E2E: `ocr_select_all` + `ocr_copy` em imagem de 2 linhas com
  pontuação → clipboard byte-idêntico ao texto original. Screenshot do
  highlight contínuo OK. **Arrasto e duplo clique seguem p/ teste manual.**

### Ajustes pós-teste visual (2026-10-30)
1. `ocr_copy` agora **só copia a seleção** — sem seleção, nada é copiado
   (log: "ocr: no selection — drag over words to select them"). Removido o
   fallback de copiar o texto completo.
2. Barra de status ("ocr: N palavras — ...") virou **debug-only**: só aparece
   com a variável de ambiente `IMV_OCR_DEBUG=1`. Verificado com screenshots
   (`grim`): sem a variável não há barra; com ela a barra aparece.
3. Idiomas: o comando `ocr <lang>` já aceita qualquer traineddata instalado.
   Documentados `por`, `spa`, `jpn`, `chi_sim` e combinações (`por+eng`) em
   `doc/imv.1.txt`, `doc/imv.5.txt` e `files/imv_config` (binds comentados).
   Pacotes Arch: `tesseract-data-spa`, `-jpn`, `-chi_sim`, `-chi_tra`, `-kor`,
   `-fra`, `-deu` (instalação com sudo pendente do usuário).

### Código (COMPILADO e testado end-to-end em 2026-10-30)
- Build limpo (`meson setup build && meson compile -C build`), zero warnings
  com `-Wall -Wextra -Wpedantic`. Único ajuste pós-escrita: `ocr.c` usava
  `TessResultIteratorBoundingBox` (inexistente na API C); trocado por
  `TessResultIteratorGetPageIteratorConst` + `TessPageIteratorBoundingBox`.
- Teste E2E em Wayland (Hyprland), via IPC (`imv-msg`):
  - Texto em inglês: `ocr` + `ocr_copy` → clipboard = "Ola mundo OCR teste 123" ✓
  - Texto em português: `ocr por` + `ocr_copy` → "Ação, coração, português ção" ✓
  - `ocr_clear` ✓
- Ambiente: `wl-copy` instalado ✓; `tesseract-data-por` já estava instalado
  (`por.traineddata` presente) — pendência de sudo resolvida.
- Falta teste **visual/manual**: botão "vidro", holofote, seleção por arrasto
  (interação com mouse não é coberta pelo teste IPC).
- Observação: `imv` cria o socket IPC com o PID do processo real; ao iniciar
  em background pelo shell, `$!` pode divergir — usar `pgrep -x imv-wayland`.
  Nunca usar `pkill -f "imv-wayland ..."` (mata o próprio shell do script).
- **meson** (`meson_options.txt`, `meson.build`): opção `tesseract` (feature),
  define `IMV_ENABLE_TESSERACT`, adiciona `src/ocr.c` ao build.
- **`src/ocr.h` / `src/ocr.c`** (novos): wrapper da API C do tesseract.
  - `imv_ocr_create(lang)` / `imv_ocr_free()`
  - `imv_ocr_run(ocr, bitmap)` → `struct imv_ocr_result` com texto completo +
    array de palavras (`text`, bbox `x,y,w,h`, `confidence`)
  - `imv_ocr_result_free()`
  - Recebe o RGBA direto do `struct imv_bitmap` (sem reler o arquivo).
- **`src/canvas.h` / `src/canvas.c`**: novo accessor `imv_canvas_cairo()`
  expondo o `cairo_t` para desenho customizado.
- **`src/imv.c`**:
  - Estado OCR no `struct imv` (engine, resultado, seleção, geometria do botão).
  - Comandos: `ocr [lang]`, `ocr_clear`, `ocr_copy`.
  - Binds padrão: `<Shift+O>` = ocr, `<Escape>` = ocr_clear, `<Ctrl+c>` = ocr_copy.
  - Render `draw_ocr_overlay()`: escurecimento com fill even-odd (efeito
    "holofote" — tudo escurece exceto as caixas das palavras), caixas de
    highlight translúcidas, seleção em azul, barra de status inferior-esquerda,
    botão "vidro" inferior-direito com o ícone SVG embutido
    (`ocr_icon_svg`, traço claro; rsvg ≥ 2.46, fallback texto "OCR").
  - Mouse: clique no botão executa/limpa OCR; arrastar seleciona palavras
    (pan por arrasto é desativado enquanto o overlay está ativo).
  - Cópia: seleção (ou texto completo) via subprocesso `wl-copy` (Wayland)
    ou `xclip -selection clipboard` (X11); quebra de linha por heurística de y.
  - Overlay limpo ao trocar de imagem; recursos liberados em `imv_free`.
- **`files/imv_config`**: binds de OCR documentados.

## Decisões tomadas

1. Motor: **libtesseract** (API C), pixels RGBA do bitmap já decodificado —
   funciona com qualquer formato e com stdin.
2. Execução **síncrona** na v1 (UI congela brevemente); assíncrono fica para depois.
3. Texto do overlay **invisível** — visível é o scan; overlay serve só para
   seleção/cópia (padrão de camada OCR de PDF). Motivo: re-renderizar o texto
   nunca alinha 100% e gerava borrões no mockup.
4. Clipboard via **subprocesso** (`wl-copy`/`xclip`) em vez de implementar
   seleção nativa Wayland/X11.
5. Caixas de palavras desenhadas apenas quando rotação = 0 e sem mirror
   (débito técnico; ver issue 001).
6. Idioma default `eng`; `:ocr por` para português (ou `por+eng`).

## O que falta fazer

1. ~~Compilar~~, ~~instalar~~, ~~testar E2E~~, ~~documentar~~, ~~commitar~~ —
   tudo feito. Commit `37a7b4f` na branch `ocr-feature` (2026-10-30).
2. Instalar pacotes de idiomas extras (`tesseract-data-spa/jpn/chi_sim/...`)
   — comando com sudo já repassado ao usuário.
3. ~~Enviar fix de `dmouse1` ao upstream~~ — usuário decidiu que é
   irrelevante no momento (não usa o imv padrão).
4. Possíveis itens da issue 001 (futuro): caixas com rotação/mirror, texto
   vertical CJK, seleção palavra-a-palavra ao arrastar após duplo clique,
   intensidades configuráveis, scanline animada, OCR assíncrono.
