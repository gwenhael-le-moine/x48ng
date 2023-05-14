#!/usr/bin/env sh

DOTX48NG=${DOTX48NG:-.x48ng}

ROM=${ROM:-@PREFIX@/share/x48ng/ROMsgxrom-r}

[ -d ~/${DOTX48NG} ] && rm -fr ~/${DOTX48NG}
mkdir -p ~/${DOTX48NG}

cp $ROM ~/${DOTX48NG}/rom.dump
cp ~/${DOTX48NG}/rom.dump ~/${DOTX48NG}/rom
cd ~/${DOTX48NG}

PORT1_SIZE=128K
PORT2_SIZE=4M

if $(echo $ROM | grep -q "^sx"); then
    PORT2_SIZE=128K
fi

@PREFIX@/share/x48ng/mkcard $PORT1_SIZE port1
@PREFIX@/share/x48ng/mkcard $PORT2_SIZE port2

cd ~/${DOTX48NG}/
@PREFIX@/bin/x48ng -home ${DOTX48NG} -verbose -initialize
