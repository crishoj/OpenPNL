(
echo "EXTRA_DIST =

include_HEADERS =      \\"
find . -iname '*.h*' | sed -e 's/^.\//     /' -e 's/$/     \\/' | sed -e '$ s/     \\$//'

) >Makefile.am

