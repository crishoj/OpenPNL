(
echo " 
LIBS = -ldl -lm

include \$(top_srcdir)/Makefile.incl

INCLUDES = \$(PNL_INCL) -I\$(srcdir)/../include

lib_LTLIBRARIES = libhigh.la

libhigh_la_SOURCES =      \\"
find . -iname '*.c*' | sed -e 's/^.\//     /' -e 's/$/     \\/' | sed -e '$ s/     \\$//'
echo "
libhigh_la_LDFLAGS = -no-undefined -version-info \$(LT_CURRENT):\$(LT_REVISION):\$(LT_AGE) -release \$(LT_RELEASE) @LD_FLAGS@
"
) >Makefile.am

