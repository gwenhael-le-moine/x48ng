mkdir ~/.x48ng
cp @PREFIX@/share/x48/ROMs/gxrom-r ~/.x48ng/rom
cp ~/.x48ng/rom ~/.x48ng/rom.dump
cd ~/.x48ng
@PREFIX@/share/x48/mkcard 128K port1
@PREFIX@/share/x48/mkcard 4M port2
x48 -initialize
