 ###########################################################################
 #  Project      _           _ _
 #     ___ _ __ (_)_ __   __| | |_   _
 #    / __| '_ \| | '_ \ / _` | | | | |
 #    \__ \ |_) | | | | | (_| | | |_| |
 #    |___/ .__/|_|_| |_|\__,_|_|\__, |
 #        |_|                    |___/
 #
 # Copyright (C) 2012, Daniel Stenberg <daniel@haxx.se>
 #
 # This software is licensed as described in the file LICENSE, which you
 # should have received as part of this distribution. The terms are also
 # available at http://spindly.haxx.se/license.html
 #
 # This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 # KIND, either express or implied.
 #
 ###########################################################################
AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include demo
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
