ACLOCAL_AMFLAGS=-I m4
SUBDIRS = lib src doc tests
 
doc_DATA = AUTHORS NEWS README COPYING

EXTRA_DIST = debian/changelog debian/compat		\
		debian/control	debian/copyright	\
		debian/rules debian/shlibs.local	\
		debian/symbols				\
		debian/libdrawtk0.install		\
		debian/libdrawtk0-dev.install		\
		debian/libdrawtk0-dev.examples	

REMOVE_DISTDIR = \
  { test ! -d "$(distdir)" \
    || { find "$(distdir)" -type d ! -perm -200 -exec chmod u+w {} ';' \
         && rm -fr "$(distdir)"; }; }

dist-deb: distdir
	am__cwd=`pwd` \
	&& cd $(distdir) \
	&& dpkg-buildpackage $(DPKG_BUILDPACKAGE_FLAGS) \
	&& cd "$$am__cwd" 
	$(REMOVE_DISTDIR)

