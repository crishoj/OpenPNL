 
LIBS = -ldl -lm

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include

lib_LTLIBRARIES = libhigh.la

libhigh_la_SOURCES =      \
     BNet.cpp     \
     BNetCB.cpp     \
     ModelEngine.cpp     \
     PersistWBNet.cpp     \
     PersistWNodeInfo.cpp     \
     TokenCover.cpp     \
     WCover.cpp     \
     WDistribFun.cpp     \
     WInner.cpp     \
     Wcsv.cpp     \
     pnlTok.cpp     \
     pnlWDistributions.cpp     \
     pnlWEvidence.cpp     \
     pnlWGraph.cpp     \
     pnlWProbabilisticNet.cpp

libhigh_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LD_FLAGS@

