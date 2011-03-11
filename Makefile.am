#----------------------------------------

SUBDIRS = src ressources

dist-hook:
	cp -Rf "$(top_srcdir)/release/ressources" $(distdir)/

total:
	make
	make install

