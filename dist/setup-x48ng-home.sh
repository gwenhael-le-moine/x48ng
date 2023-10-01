#!/usr/bin/env sh

DOTX48NG=${DOTX48NG:-.config/x48ng}

ROM=${ROM:-gxrom-r}

[ -d ~/${DOTX48NG} ] && rm -fr ~/${DOTX48NG}
mkdir -p ~/${DOTX48NG}

x48ng --print-config > ~/${DOTX48NG}/config.lua

cp @PREFIX@/share/x48ng/ROMs/$ROM ~/${DOTX48NG}/rom
cd ~/${DOTX48NG}

PORT1_SIZE=128K
PORT2_SIZE=4M

if $(echo $ROM | grep -q "^sx"); then
    PORT2_SIZE=128K
fi

@PREFIX@/share/x48ng/mkcard $PORT1_SIZE port1
@PREFIX@/share/x48ng/mkcard $PORT2_SIZE port2
