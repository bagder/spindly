AUTOMAKE_OPTIONS = foreign

SUBDIRS = src includes
DIST_SUBDIRS = doc tests bin

EXTRA_DIST = TODO README LICENSE

ACLOCAL_AMFLAGS = -I m4

.PHONY: bin

check:
	@(cd tests; $(MAKE) run)

bin:
	@(cd bin; $(MAKE))
