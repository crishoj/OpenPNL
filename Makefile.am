# The top-level input Makefile for PNL

## Any directories that you want built and installed should go here.
SUBDIRS = cxcore trs c_pgmtk doc high

am__configure_deps=@am__configure_deps@
## Any directories you want a part of the distribution should be listed
## here, as well as have a Makefile generated at the end of configure.in
##
## This only works for subdirectories one level deep.
#DIST_SUBDIRS = $(SUBDIRS)

# pnl runtime configuration script
#bin_SCRIPTS = pnl-config

# All the rest of the distributed files
EXTRA_DIST = configure.gcc configure.icc \
             changes.txt readme.txt pnlLicense.txt 

# Rule to build tar-gzipped distribution package
#$(PACKAGE)-$(VERSION).tar.gz: dist

#dist-hook:
#	rm -rf `find $(distdir) -name CVS`

# Rule to build RPM distribution package
#rpm: $(PACKAGE)-$(VERSION).tar.gz
#	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule to force automake to rebuild the library
#changed:
#	@echo "This build target is no longer necessary"

# Rule to install the libraries only - prevent rebuilding apps
#install-lib:
#	cd src && $(MAKE) install-libLTLIBRARIES
