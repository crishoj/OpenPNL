CC=gcc
(
echo "
bin_PROGRAMS=tests_pnl_c

include \$(top_srcdir)/Makefile.incl

INCLUDES=\$(PNL_INCL) -I../include -I\$(top_srcdir)/trs/include
tests_pnl_c_SOURCES=\\"
ls *.c* \
| grep -v -e MATLAB -e TJTreeInference\\.cpp -e DSLPNLConverter \
| sed -e 's/^/     /' -e 's/$/     \\/' \
| sed -e '$ s/     \\$//'
echo '
tests_pnl_c_LDADD=$(PNL_LIBS) $(top_srcdir)/trs/src/libtrs.a
'
) >Makefile.am
