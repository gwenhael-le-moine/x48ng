#!/usr/bin/env sh

cd "$(dirname "$0")" ; CWD=$(pwd)

CONFIG_HOME=${XDG_CONFIG_HOME:-$HOME/.config}
DOTX48NG=${DOTX48NG:-$CONFIG_HOME/x48ng}
CONFIG_FILE="${DOTX48NG}"/config.lua
ROM=${ROM:-./ROMs/gxrom-r}

mkdir -p "${DOTX48NG}"

[ -e "${CONFIG_FILE}" ] && mv "${CONFIG_FILE}" "${CONFIG_FILE}".orig
x48ng --print-config > "${CONFIG_FILE}"

cp -r ./ROMs/ "${DOTX48NG}"/

cd "${DOTX48NG}"/ROMs/ || exit 1
echo "The next step will download all available HP 48 ROMs from https://hpcalc.org where \"HP graciously began allowing this to be downloaded in mid-2000.\""
echo "You can hit Ctrl-C now if you do not wish to download them."
read -r
make get-roms

cd "${DOTX48NG}" || exit 1
[ -e rom ] && mv rom rom.orig
ln -s "$ROM" rom

PORT1_SIZE=128
PORT2_SIZE=4096

if echo "$ROM" | grep -q "^sx"; then
    PORT2_SIZE=128
fi

[ -e port1 ] && mv port1 port1.orig
dd if=/dev/zero of="$DOTX48NG"/port1 bs=1k count=$PORT1_SIZE

[ -e port2 ] && mv port2 port2.orig
dd if=/dev/zero of="$DOTX48NG"/port2 bs=1k count=$PORT2_SIZE
