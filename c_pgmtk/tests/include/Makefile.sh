(
echo "
noinst_HEADERS =     \\"
ls *.h* \
| grep -v -e MATLAB -e DSLPNLConverter \
| sed -e 's/^/     /' -e 's/$/     \\/' \
| sed -e '$ s/     \\$//'
) >Makefile.am
