CC=gcc
(
echo "
check_PROGRAMS = tests_pnl_c

include \$(top_srcdir)/Makefile.incl

INCLUDES = \$(PNL_INCL) -I\$(srcdir)/../include -I\$(top_srcdir)/trs/include

tests_pnl_c_SOURCES =     \\"
ls *.c* \
| grep -v -e MATLAB -e TJTreeInference\\.cpp -e DSLPNLConverter \
| sed -e 's/^/     /' -e 's/$/     \\/' \
| sed -e '$ s/     \\$//'
echo '
tests_pnl_c_LDADD = $(PNL_LIBS) $(top_builddir)/trs/src/libtrs.a

TESTS = run_pnlcheck

run_pnlcheck: tests_pnl_c
	@echo "./tests_pnl_c -R FAIL -s $(top_builddir)/checkfails.sum -m -B" >$@; \
	chmod a+x $@
'
) >Makefile.am
