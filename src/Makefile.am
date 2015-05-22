AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
LDFLAGS= -lboost_regex -lboost_thread
bin_PROGRAMS = pfailover

pfailover_SOURCES = conf.cpp conf.h monitor.cpp monitor.h ipc.cpp ipc.h pfailover.cpp pfailover.h
