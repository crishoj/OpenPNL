(
echo "EXTRA_DIST = 

nobase_include_HEADERS =      \\"
find . -name '*.h*' | sed -e 's/^.\//     /' -e 's/$/     \\/' | sed -e '$ s/     \\$//'
) >Makefile.am

