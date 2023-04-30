mkdir -p ~/.x48ng

cp @PREFIX@/share/x48ng/ROMs/gxrom-r ~/.x48ng/rom.dump
cp ~/.x48ng/rom.dump ~/.x48ng/rom

cd ~/.x48ng

@PREFIX@/share/x48ng/mkcard 128K port1
@PREFIX@/share/x48ng/mkcard 4M port2

@PREFIX@/bin/x48ng -initialize
