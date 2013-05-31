#! /bin/sh
ACLOCAL=${ACLOCAL:=aclocal-1.9}
AUTOHEADER=${AUTOHEADER:=autoheader}
AUTOMAKE=${AUTOMAKE:=automake-1.9}
AUTOCONF=${AUTOCONF:=autoconf}
AUTOPOINT=${AUTOPOINT:=autopoint}
LIBTOOLIZE=${LIBTOOLIZE:=libtoolize}

echo use aclocal: $ACLOCAL
echo use autoheader: $AUTOHEADER
echo use automake: $AUTOMAKE
echo use autoconf: $AUTOCONF
echo use autopoint: $AUTOPOINT
echo use libtoolize: $LIBTOOLIZE

$ACLOCAL --version | \
   awk -vPROG="aclocal" -vVERS=1.9\
   '{if ($1 == PROG) {gsub ("-.*","",$4); if ($4 < VERS) print PROG" < version "VERS"\nThis may result in errors\n"}}'
   
$AUTOMAKE --version | \
   awk -vPROG="automake" -vVERS=1.9\
   '{if ($1 == PROG) {gsub ("-.*","",$4); if ($4 < VERS) print PROG" < version "VERS"\nThis may result in errors\n"}}'
   
$AUTOPOINT --force && \
$LIBTOOLIZE --force && \
$ACLOCAL -I m4 && \
$AUTOHEADER && \
$AUTOMAKE --gnu --add-missing && \
$AUTOCONF && \
./configure $@
