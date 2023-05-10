#!/usr/bin/env sh

TARGET=${TARGET:-.x48ng}

mkdir -p ~/${TARGET}

cp @PREFIX@/share/x48ng/ROMs/gxrom-r ~/${TARGET}/rom.dump
cp ~/${TARGET}/rom.dump ~/${TARGET}/rom

cd ~/${TARGET}

@PREFIX@/share/x48ng/mkcard 128K port1
@PREFIX@/share/x48ng/mkcard 4M port2

@PREFIX@/bin/x48ng -initialize -home ${TARGET}
