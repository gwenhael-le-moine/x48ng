AUTOMAKE_OPTIONS = dejagnu

SUBDIRS = src romdump

EXTRA_DIST=

X48 : src/X48.ad
	cp $< $@

appdefault_DATA = X48

DISTCHECK_CONFIGURE_FLAGS = --with-appdefaultdir='$${datadir}/X11/app-defaults'
