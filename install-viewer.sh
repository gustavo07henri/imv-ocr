#!/bin/sh
# Instala (ou reinstala) o imv+OCR como visualizador padrao do sistema,
# em ~/.local — nivel de usuario, sem sudo para instalar, e fora do
# alcance de atualizacoes do pacote imv da distro (que so toca /usr).
#
# Idempotente: rode novamente apos `git pull`, reinstalacao do sistema,
# ou se suspeitar que o visualizador padrao voltou ao imv original.
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PREFIX="$HOME/.local"

echo "==> Compilando imv+OCR (prefixo: $PREFIX)"
if [ ! -d "$SCRIPT_DIR/build" ]; then
  meson setup "$SCRIPT_DIR/build" --prefix "$PREFIX"
else
  meson setup "$SCRIPT_DIR/build" --reconfigure --prefix "$PREFIX"
fi
meson compile -C "$SCRIPT_DIR/build"
meson install -C "$SCRIPT_DIR/build"

# Remove instalacao antiga em /usr/local, se existir (precisa de sudo)
if [ -e /usr/local/bin/imv-wayland ]; then
  echo "==> Removendo instalacao antiga em /usr/local (sudo)"
  sudo rm -f /usr/local/bin/imv /usr/local/bin/imv-wayland \
    /usr/local/bin/imv-x11 /usr/local/bin/imv-msg \
    /usr/local/share/applications/imv.desktop \
    /usr/local/share/applications/imv-dir.desktop \
    /usr/local/etc/imv_config \
    /usr/local/share/man/man1/imv.1* /usr/local/share/man/man1/imv-msg.1* \
    /usr/local/share/man/man5/imv.5*
fi

echo
echo "==> Instalado. 'imv' resolve para: $(command -v imv)"
echo "    (se o terminal mostrar outro caminho, rode: hash -r)"
echo "==> Para desfazer: $SCRIPT_DIR/rollback-viewer.sh"
