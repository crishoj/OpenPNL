(
echo 'SUBDIRS = include

LIBS = -ldl -lm

include $(top_srcdir)/Makefile.incl

SOMAJOR=@PNLMAJOR@
SOMINOR=@PNLMINOR@
SOSUBMINOR=@PNLSUBMINOR@
INCLUDES=$(PNL_INCL)
'
echo "
lib_LTLIBRARIES=libpnl.la

libpnl_la_SOURCES=      \\"
ls *.c* | grep -v -e pnlMlLearningEngine\\.cpp -e pnlCreate.\*\\.cpp | sed -e 's/^/     /' -e 's/$/     \\/' | sed -e '$ s/     \\$//'

echo '
libpnl_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LD_FLAGS@
'

) >Makefile.am

