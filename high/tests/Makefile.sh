(
echo "
check_PROGRAMS = testMultivariate testLIMID testDBN

include \$(top_srcdir)/Makefile.incl

INCLUDES = \$(PNL_INCL) -I\$(srcdir)/../include -I\$(srcdir)/../examples -I\$(srcdir)

testMultivariate_SOURCES = Test.cpp
testMultivariate_LDADD = \$(PNL_LIBS) \$(top_builddir)/high/source/libhigh.la

testLIMID_SOURCES = TestLimid.cpp
testLIMID_LDADD = \$(PNL_LIBS) \$(top_builddir)/high/source/libhigh.la

testDBN_SOURCES = TestsDBN.cpp
testDBN_LDADD = \$(PNL_LIBS) \$(top_builddir)/high/source/libhigh.la
"
) >Makefile.am

