(
echo "
bin_PROGRAMS = example

include \$(top_srcdir)/Makefile.incl

INCLUDES = \$(PNL_INCL) -I\$(srcdir)/../include -I\$(srcdir)
example_SOURCES = rps.cpp console.cpp example_common_funs.cpp script_lex.cpp
example_LDADD = \$(PNL_LIBS) \$(top_builddir)/high/source/libhigh.la

highexdir = \$(prefix)/high/examples

nobase_highex_DATA =     \\"
patterns="*.cpp *.hpp *.pnl *.csv *.xml"
for i in $patterns
do
    ls $i | sed -e 's/^/     /' -e 's/$/     \\/'
done | sed -e '$ s/     \\$//'
) >Makefile.am

