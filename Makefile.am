AUTOMAKE_OPTIONS = foreign

SUBDIRS = src data

install-data-local:
	mkdir $(sysconfdir)/pfailover
	cp -n $(srcdir)/data/pfailover.conf $(sysconfdir)/pfailover/
	cp -n $(srcdir)/data/example.* $(sysconfdir)/pfailover/
