
bin_PROGRAMS = example

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include -I$(srcdir)
example_SOURCES = rps.cpp console.cpp example_common_funs.cpp script_lex.cpp
example_LDADD = $(PNL_LIBS) $(top_builddir)/high/source/libhigh.la

highexdir = $(prefix)/high/examples

nobase_highex_DATA =     \
     console.cpp     \
     example_common_funs.cpp     \
     rps.cpp     \
     script_lex.cpp     \
     console.hpp     \
     example_common_funs.hpp     \
     script_lex.hpp     \
     AddDeleteNodes.pnl     \
     checkSL.pnl     \
     ContinuosEvidence.pnl     \
     gaussian.pnl     \
     MultiGaussian.pnl     \
     Numeration.pnl     \
     rpsCreate.pnl     \
     scripting.pnl     \
     testSetP.pnl     \
     TopolSort2.pnl     \
     TopolSort.pnl     \
     rpsLearn.csv     \
     SLDataRPS.csv     \
     bnet0.xml
