/* imv + OCR — mockup de design
 *
 * Simula o fluxo planejado para o imv:
 *   1. usuário aciona o botão (ou tecla O)
 *   2. estado "processando" (spinner + linha de varredura)
 *   3. resultado: imagem escurece, cada bloco de texto detectado
 *      recebe um overlay selecionável com fundo re-clareado
 */

const stage     = document.getElementById('stage');
const page      = document.getElementById('page');
const ocrLayer  = document.getElementById('ocrLayer');
const scanline  = document.getElementById('scanline');
const btn       = document.getElementById('ocrBtn');
const statusbar = document.getElementById('statusbar');

// #result aplica o estado final sem espera nem animações (screenshots de validação)
const OCR_DELAY_MS = location.hash === '#result' ? 0 : 2000; // simula o tempo do tesseract
if (OCR_DELAY_MS === 0) {
  const st = document.createElement('style');
  st.textContent = '* { transition: none !important; animation: none !important; }';
  document.head.appendChild(st);
}
let state = 'idle';        // idle | processing | done

function setStatus(msg) {
  statusbar.textContent = msg;
  statusbar.hidden = !msg;
}

function setProcessing(on) {
  btn.querySelector('.icon').hidden = on;
  btn.querySelector('.spinner').hidden = !on;
  scanline.hidden = !on;
}

/* Posiciona um overlay sobre cada bloco .t da página.
 * Equivale ao que o imv fará com as bounding boxes do tesseract. */
function buildOverlay() {
  ocrLayer.textContent = '';
  const stageRect = stage.getBoundingClientRect();
  const blocks = page.querySelectorAll('.t');
  let count = 0;

  blocks.forEach((el) => {
    const text = el.textContent.trim();
    if (!text || text === ' ') return;

    const r = el.getBoundingClientRect();
    if (r.width < 2 || r.height < 2) return;

    const line = document.createElement('div');
    line.className = 'ocr-line';
    line.textContent = el.textContent;

    const cs = getComputedStyle(el);
    line.style.left       = (r.left - stageRect.left) + 'px';
    line.style.top        = (r.top - stageRect.top) + 'px';
    line.style.width      = r.width + 'px';
    line.style.height     = r.height + 'px';
    line.style.fontSize   = cs.fontSize;
    line.style.fontWeight = cs.fontWeight;
    line.style.lineHeight = cs.lineHeight;
    line.style.textAlign  = cs.textAlign;
    line.style.animationDelay = (count * 30) + 'ms';

    ocrLayer.appendChild(line);
    count++;
  });

  return count;
}

function showResult() {
  setProcessing(false);
  const count = buildOverlay();

  page.classList.add('dimmed');
  ocrLayer.hidden = false;
  btn.classList.add('done');
  state = 'done';

  setStatus(`ocr: ${count} blocos de texto detectados — arraste para selecionar, Ctrl+C para copiar · Esc para limpar`);
}

function runOcr() {
  state = 'processing';
  setProcessing(true);
  setStatus('ocr: processando imagem…');

  if (OCR_DELAY_MS === 0) {
    showResult(); // modo screenshot: estado final síncrono
  } else {
    setTimeout(showResult, OCR_DELAY_MS);
  }
}

function clearOcr() {
  ocrLayer.hidden = true;
  ocrLayer.textContent = '';
  page.classList.remove('dimmed');
  btn.classList.remove('done');
  setStatus('');
  state = 'idle';
}

btn.addEventListener('click', () => {
  if (state === 'idle') runOcr();
  else if (state === 'done') clearOcr();
});

document.addEventListener('keydown', (ev) => {
  if (ev.key === 'o' || ev.key === 'O') {
    if (state === 'idle') runOcr();
    else if (state === 'done') clearOcr();
  } else if (ev.key === 'Escape' && state !== 'idle') {
    clearOcr();
  }
});

/* Mantém o overlay alinhado ao redimensionar a janela */
window.addEventListener('resize', () => {
  if (state === 'done') buildOverlay();
});

/* Atalho para testes/screenshots: abrir com #result já mostra o
 * estado pós-OCR, sem esperar o delay simulado */
if (location.hash === '#result') {
  runOcr();
}
