# imv + OCR

**Fork** do [imv](https://sr.ht/~exec64/imv) (visualizador de imagens
minimalista para Wayland/X11, de **Harry Jeffery e contribuidores**) com uma
camada de **OCR interativa**: reconhece o texto da imagem aberta e permite
selecioná-lo e copiá-lo como em um editor de texto ou na camada de texto de
um PDF.

Todo o crédito do visualizador original vai para o autor e contribuidores do
imv — este fork apenas adiciona a feature de OCR (baseada no **v5.0.1**) e um
fix de eventos de mouse no backend Wayland. Desenvolvido com auxílio de IA
(Kimi Code CLI).

> **Status:** versão inicial, pouco testada — validada apenas em um sistema
> **Omarchy 4.0.4** (Arch Linux + Hyprland/Wayland). Pode não funcionar em
> outros ambientes. Use por sua conta e risco.

## Motivação

Screenshots, fotos de documentos, memes com texto, slides — constantemente
queremos copiar um trecho de texto que está preso dentro de uma imagem. O
fluxo tradicional é abrir um programa de OCR separado, exportar, copiar. Com
este fork, o visualizador padrão de imagens já faz isso no lugar: abriu,
`Shift+O`, arrastou, `Ctrl+C`.

## Como funciona

- O `tesseract` (API C) roda sobre os pixels RGBA já decodificados pelo imv —
  funciona com qualquer formato que o imv abre (png, jpeg, webp, gif, heic,
  jxl, tiff, svg...) e até com imagens vindas de stdin.
- As caixas por palavra **e por caractere** (`RIL_SYMBOL`) são extraídas,
  com fronteiras de palavra/linha fornecidas pelo próprio tesseract.
- O overlay escurece a imagem exceto as áreas de texto (efeito holofote) e a
  seleção é desenhada como highlight contínuo por linha.

## Uso

| Ação | Efeito |
|---|---|
| `Shift+O` ou botão inferior-direito | Liga/desliga o OCR |
| Arrastar o mouse | Seleção caractere a caractere (padrão editor) |
| Duplo clique | Seleciona a palavra |
| Triplo clique | Seleciona a linha |
| `Ctrl+A` | Seleciona todo o texto reconhecido |
| `Ctrl+C` | Copia a seleção (`wl-copy` no Wayland, `xclip` no X11) |
| `Esc` | 1ª vez: solta a seleção · 2ª vez: limpa o overlay |

Comandos de console (`:`) e IPC (`imv-msg <pid> <cmd>`):

- `ocr [lang]` — roda o OCR; idioma padrão `eng`. Ex.: `ocr por`, `ocr jpn`,
  `ocr chi_sim`, `ocr por+eng`
- `ocr_select_all` — seleciona tudo
- `ocr_copy` — copia a seleção (sem seleção, não faz nada)
- `ocr_clear` — solta a seleção / limpa o overlay

Diagnóstico: rode com `IMV_OCR_DEBUG=1 imv imagem.png` para ver a barra de
status (contagem de palavras/caracteres) e logs de press/drag/release no
stderr.

## Instalação

Dependências de build: as do imv original (`meson`, `ninja`, `pango`, `cairo`,
`inih`, backend(s) de janela) mais `tesseract` — detectado automaticamente;
para forçar: `meson setup build -Dtesseract=enabled`.

Dependências de runtime: dados de idioma do tesseract e uma ferramenta de
clipboard:

```bash
# Arch Linux — idiomas (eng e por já costumam vir com o tesseract)
sudo pacman -S tesseract-data-por tesseract-data-spa \
  tesseract-data-jpn tesseract-data-chi_sim
# clipboard no Wayland
sudo pacman -S wl-clipboard
```

Instalação como visualizador padrão do usuário (em `~/.local`, sem sudo):

```bash
git clone <este-repo> && cd imv-change-ocr
./install-viewer.sh
```

O script é **idempotente**: rode-o novamente após `git pull`, após trocar de
máquina, ou se o visualizador padrão parecer ter voltado ao imv da distro.
Como a instalação fica em `~/.local` (que precede `/usr/bin` no PATH e
`/usr/share` no `XDG_DATA_DIRS`), atualizações do pacote `imv` da distro
**não afetam** esta versão — o pacote original permanece intacto como
fallback.

### Rollback

```bash
./rollback-viewer.sh
```

Remove os arquivos de `~/.local` (e restos em `/usr/local`, se houver) e o
sistema volta imediatamente ao `imv` da distro.

## Ferramentas e stack

- **C99** + [meson](https://mesonbuild.com)/ninja (build do imv original)
- **[tesseract](https://github.com/tesseract-ocr/tesseract)** 5 + leptonica — motor OCR
- **cairo/pango** — overlay e renderização de UI
- **Wayland** (wlroots/XDG shell) e X11 — backends de janela
- **wl-clipboard / xclip** — cópia para o clipboard
- `IMV_OCR_DEBUG=1` — instrumentação de diagnóstico da seleção

## Estrutura do fork

```
src/ocr.c, src/ocr.h     wrapper da API C do tesseract (palavras + símbolos)
src/imv.c                estado OCR, comandos, seleção por carets, overlay
src/wl_window.c          fix: dmouse1 nunca era limpo (bug latente upstream)
files/imv_config         binds OCR documentados
install-viewer.sh        instala/reinstala em ~/.local (idempotente)
rollback-viewer.sh       volta ao imv da distro
docs/STATUS.md           histórico de decisões e testes do desenvolvimento
docs/issues/             melhorias futuras (overlay visual)
mockup/                  protótipo HTML/CSS/JS do overlay
```

## Limitações conhecidas

- Caixas de texto e seleção só com rotação 0 e sem espelhamento
  (`docs/issues/001-overlay-ocr-visual.md`)
- Texto vertical CJK não é tratado
- OCR é síncrono (a UI congela por um instante em imagens grandes)

## Licença

MIT, como o imv original. Ver `LICENSE`.
