(
echo "
check_PROGRAMS = test_high

include \$(top_srcdir)/Makefile.incl

INCLUDES = \$(PNL_INCL) -I\$(srcdir)/../include -I\$(srcdir)/../examples -I\$(srcdir)

test_high_SOURCES =     \\"
ls *.cpp | sed -e 's/^/     /' -e 's/$/     \\/' -e '$ s/     \\$//'
echo "
test_high_LDADD = \$(PNL_LIBS) \$(top_builddir)/high/source/libhigh.la
"
) >Makefile.am

