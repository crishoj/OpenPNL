(
echo "
SUBDIRS = Data ex_inf_learn_dbn ex_param gibbs inf_learn_bnet learn_param \\
          linux_test mixture_gaussian_bnet parPNLTest testLIMID testSL \\
          testSoftMax trial use_matrix

pnlexamplesdir = \$(prefix)/examples

nobase_pnlexamples_DATA =      \\"
ls Data | sed -e '/Makefile.*/d' -e '/CVS/d' -e 's/^.*$/     Data\/&     \\/'
patterns="*.[ch]*"
for i in $patterns
do
    find . -iname $i | sed -e 's/^.\//     /' -e 's/$/     \\/' 
done \
| sed -e '$ s/     \\$//'
) >Makefile.am
