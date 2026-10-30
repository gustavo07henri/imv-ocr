#!/bin/sh
# Rollback: remove o imv+OCR (instalado em ~/.local e, se houver, restos
# em /usr/local) e devolve o visualizador ao imv da distro (pacote pacman
# em /usr/bin/imv), que nunca foi alterado.
# ~/.local/bin precede /usr/bin no PATH; removidos os arquivos locais,
# o sistema volta automaticamente ao pacote original.
set -e

echo "==> Removendo imv+OCR de ~/.local"
rm -f "$HOME/.local/bin/imv" "$HOME/.local/bin/imv-wayland" \
  "$HOME/.local/bin/imv-x11" "$HOME/.local/bin/imv-msg" \
  "$HOME/.local/share/applications/imv.desktop" \
  "$HOME/.local/share/applications/imv-dir.desktop" \
  "$HOME/.local/etc/imv_config" \
  "$HOME/.local/share/man/man1/imv.1"* \
  "$HOME/.local/share/man/man1/imv-msg.1"* \
  "$HOME/.local/share/man/man5/imv.5"*

if [ -e /usr/local/bin/imv-wayland ]; then
  echo "==> Removendo restos em /usr/local (sudo)"
  sudo rm -f /usr/local/bin/imv /usr/local/bin/imv-wayland \
    /usr/local/bin/imv-x11 /usr/local/bin/imv-msg \
    /usr/local/share/applications/imv.desktop \
    /usr/local/share/applications/imv-dir.desktop \
    /usr/local/etc/imv_config \
    /usr/local/share/man/man1/imv.1* /usr/local/share/man/man1/imv-msg.1* \
    /usr/local/share/man/man5/imv.5*
fi

echo
echo "==> Rollback concluido. 'imv' agora resolve para: $(command -v imv)"
echo "    (se o terminal mostrar outro caminho, rode: hash -r)"
