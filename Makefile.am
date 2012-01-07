AUTOMAKE_OPTIONS = foreign

SUBDIRS = src includes
DIST_SUBDIRS = $(SUBDIRS) doc tests bin

EXTRA_DIST = TODO README LICENSE

ACLOCAL_AMFLAGS = -I m4

.PHONY: bin

check:
	@(cd tests; $(MAKE) run)

bin:
	@(cd bin; $(MAKE))

# a hook to rename [file].dist to [file]
dist-hook:
	find $(distdir) -name "*.dist" -exec rm {} \;
	(distit=`find $(srcdir) -name "*.dist"`; \
	for file in $$distit; do \
	  strip=`echo $$file | sed -e s/^$(srcdir)// -e s/\.dist//`; \
	  cp $$file $(distdir)$$strip; \
	done)
