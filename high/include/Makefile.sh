(
echo "EXTRA_DIST = 

highincludedir = \$(prefix)/high/include

nobase_highinclude_HEADERS =      \\"
find . -iname '*.h*' | sed -e 's/^.\//     /' -e 's/$/     \\/' | sed -e '$ s/     \\$//'
) >Makefile.am

