#!/usr/bin/env bash

cd "$(dirname "$0")" || exit 1
CWD=$(pwd)

CONFIG_HOME=${XDG_CONFIG_HOME:-$HOME/.config}
DOTX48NG=${DOTX48NG:-$CONFIG_HOME/x48ng}
CONFIG=${CONFIG:-config.lua}
ROM=${ROM:-ROMs/gxrom-r}
RAM=${RAM:-ram}
STATE=${STATE:-state}
PORT1=${PORT1:-port1}
PORT2=${PORT2:-port2}

mkdir -p "${DOTX48NG}"
cd "${DOTX48NG}" || exit 1

cp -R "$CWD"/ROMs "${DOTX48NG}"/ROMs

PORT1_SIZE=128
PORT2_SIZE=4096
if echo "$ROM" | grep -q "sxrom"; then
    PORT2_SIZE=128
fi

[ -e ram ] && mv ram ram.orig
[ -e state ] && mv state state.orig
[ -e port1 ] && mv port1 port1.orig
[ -e port2 ] && mv port2 port2.orig
[ -e "${CONFIG}" ] && mv "${CONFIG}" "${CONFIG}".orig

dd if=/dev/zero of="$DOTX48NG"/"$PORT1" bs=1k count=$PORT1_SIZE
dd if=/dev/zero of="$DOTX48NG"/"$PORT2" bs=1k count=$PORT2_SIZE

echo "The next step will download all available HP 48 ROMs from https://hpcalc.org where \"HP graciously began allowing this to be downloaded in mid-2000.\""
echo "You can hit Ctrl-C now if you do not wish to download them."
read -r
gmake -C ROMs get-roms

BINX48NG=x48ng
[ -x "$CWD"/x48ng ] && BINX48NG="$CWD"/x48ng

"$BINX48NG" --rom="$ROM" --ram="$RAM" --state="$STATE" --port1="$PORT1" --port2="$PORT2" --print-config > "${CONFIG}"
