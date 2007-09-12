
check_PROGRAMS = test_high

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include -I$(srcdir)/../examples -I$(srcdir) -I$(srcdir)/../../trs/include

test_high_SOURCES =     \
     Test.cpp     \
     TestCommon.cpp     \
     TestCondGaussian.cpp     \
     TestGaussianBNet.cpp     \
     TestLimid.cpp     \
     TestMRF.cpp     \
     TestNumeration.cpp     \
     TestSoftMax.cpp     \
     TestsDBN.cpp     \
     tsys.cpp

test_high_LDADD = $(PNL_LIBS) $(top_builddir)/high/source/libhigh.la $(top_builddir)/trs/src/libtrs.a

