SUBDIRS = po

bin_PROGRAMS=cif
cif_SOURCES=cif-opt.c cif-opt.h cif.c xmalloc.c system.h cif_dummy.c aclocal.m4
cif_LDADD=-lopts -lrt libcif.la libcif_dps.la
cif_CFLAGS=-W -Wall
nobase_include_HEADERS=cif.h cif_dps.h

cif-opt.c cif-opt.h: $(srcdir)/cif-opt.def
	autogen -L/usr/share/autogen $(srcdir)/cif-opt.def

cif.1: $(srcdir)/cif-opt.def
	autogen -T manpage.tpl  $(srcdir)/cif-opt.def
	awk -f updater cif.1 > tmp$$$$ && mv tmp$$$$ cif.1
	rm updater

lib_LTLIBRARIES=libcif.la libcif_dps.la libcif_dn.la

libcif_la_SOURCES=cif_api.c cif_intern.h
libcif_la_CFLAGS=-W -Wall

libcif_dps_la_SOURCES=cif_dps.c cif_dps.h
libcif_dps_la_CFLAGS=-W -Wall

libcif_dn_la_SOURCES=cif_dn.c cif_dn.h
libcif_dn_la_CFLAGS=-W -Wall

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

EXTRA_DIST=cif.lsm.in cif.spec.in cif-opt.def cif.1 manpage.tpl cif.dox.in \
	autogen.sh

#  build and install the .info pages
# info_TEXINFOS = cif.texinfo
# cif_TEXINFOS = gpl.texinfo

#  install the man pages
man_MANS=cif.1

DISTCLEANFILES = *~ 

MAINTAINERCLEANFILES = acconfig.h aclocal.m4 ABOUT-NLS compile \
                       config.guess config.h.in config.rpath config.sub \
                       configure depcomp INSTALL install-sh \
		       intltool-extract.in intltool-merge.in \
                       intltool-update.in ltmain.sh ltconfig Makefile.in \
                       missing mkinstalldirs texinfo.tex cif-opt.c cif-opt.h
