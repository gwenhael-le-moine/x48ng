#!/usr/bin/env sh

DOTX48NG=${DOTX48NG:-.config/x48ng}
CONFIG_FILE=~/"${DOTX48NG}"/config.lua
ROM=${ROM:-./ROMs/gxrom-r}

# [ -d ~/"${DOTX48NG}" ] && rm -fr ~"/${DOTX48NG}"
mkdir -p ~/"${DOTX48NG}"

[ -e "${CONFIG_FILE}" ] && mv "${CONFIG_FILE}" "${CONFIG_FILE}".orig
x48ng --print-config > "${CONFIG_FILE}"

cp -r @PREFIX@/share/x48ng/ROMs/ ~/"${DOTX48NG}"/

cd ~/"${DOTX48NG}"/ROMs/ || exit 1
make get-roms

cd ~/"${DOTX48NG}" || exit 1
[ -e rom ] && mv rom rom.orig
ln -s "$ROM" rom

PORT1_SIZE=128K
PORT2_SIZE=4M

if $(echo "$ROM" | grep -q "^sx"); then
    PORT2_SIZE=128K
fi

[ -e port1 ] && mv port1 port1.orig
@PREFIX@/share/x48ng/mkcard $PORT1_SIZE port1

[ -e port2 ] && mv port2 port2.orig
@PREFIX@/share/x48ng/mkcard $PORT2_SIZE port2
