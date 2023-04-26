mkdir ~/.hp48
cp /usr/share/x48/ROMs/gxrom-r ~/.hp48/rom.dump
cd ~/.hp48
/usr/share/x48/mkcard 128K port1
/usr/share/x48/mkcard 4M port2
x48 -initialize
