
check_PROGRAMS = test_high

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include -I$(srcdir)/../examples -I$(srcdir)

test_high_SOURCES =     \
     TestCommon.cpp     \
     Test.cpp     \
     TestGaussianBNet.cpp     \
     TestLimid.cpp     \
     TestMain.cpp     \
     TestMRF.cpp     \
     TestNumeration.cpp     \
     TestsDBN.cpp

test_high_LDADD = $(PNL_LIBS) $(top_builddir)/high/source/libhigh.la

