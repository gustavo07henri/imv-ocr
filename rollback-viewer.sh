#!/bin/sh
# Rollback: remove a versao OCR (instalada em /usr/local) e devolve o
# visualizador ao imv da distro (pacote pacman em /usr/bin/imv), que nunca
# foi alterado. /usr/local tem precedencia no PATH e no XDG_DATA_DIRS,
# entao basta desinstalar os arquivos locais.
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

if [ ! -f "$SCRIPT_DIR/build/meson-logs/install_log.txt" ]; then
  echo "Nada instalado (install_log.txt nao encontrado). Nada a fazer."
  exit 0
fi

sudo ninja -C "$SCRIPT_DIR/build" uninstall
rm -f "$SCRIPT_DIR/build/meson-logs/install_log.txt"

echo
echo "Rollback concluido."
echo "imv agora resolve para: $(command -v imv)"
echo "Se o terminal ainda abrir a versao antiga, rode: hash -r"
