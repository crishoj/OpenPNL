 
LIBS = -ldl -lm

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include

lib_LTLIBRARIES = libhigh.la

libhigh_la_SOURCES =      \
     BNet.cpp     \
     BNetCB.cpp     \
     DBN.cpp     \
     DBNCB.cpp     \
     LIMID.cpp     \
     LIMIDCB.cpp     \
     MRF.cpp     \
     MRFCB.cpp     \
     ModelEngine.cpp     \
     NetCallBack.cpp     \
     PersistProbNet.cpp     \
     PersistWNodeInfo.cpp     \
     PersistWTypes.cpp     \
     TokenCover.cpp     \
     Tokens.cpp     \
     WCliques.cpp     \
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

