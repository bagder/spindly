AUTOMAKE_OPTIONS = foreign

SUBDIRS = src
DIST_SUBDIRS = doc includes tests bin

EXTRA_DIST = TODO README LICENSE

ACLOCAL_AMFLAGS = -I m4

check:
	@(cd tests; $(MAKE) run)
