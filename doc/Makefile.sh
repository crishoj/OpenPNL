(
echo "
EXTRA_DIST = PNLDocHTM PNL.pdf

install-data-local: install-pnldoc

install-pnldoc:
	docdist=\$(prefix)/doc; \\
	\$(mkdir_p) \$\$docdist; \\
	cp -pR \$(srcdir)/* \$\$docdist; \\
	rm -f \$\$docdist/Makefile*

uninstall-local: uninstall-pnldoc

uninstall-pnldoc:
	docdist=\$(prefix)/doc; \\
	rm -rf \$\$docdist
"
) >Makefile.am
