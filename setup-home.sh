mkdir ~/.hp48
cp PREFIX/share/x48/ROMs/gxrom-r ~/.hp48/rom.dump
cd ~/.hp48
PREFIX/share/x48/mkcard 128K port1
PREFIX/share/x48/mkcard 4M port2
x48 -initialize
