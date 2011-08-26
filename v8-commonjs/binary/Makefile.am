#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# Binary modules
moduledir = $(libdir)/v8-commonjs/binary
module_LTLIBRARIES = \
	b.la
b_la_SOURCES = \
	b/binary.cc \
	b/binary.h \
	b/byte-array.cc \
	b/byte-array.h \
	b/byte-string.cc \
	b/byte-string.h \
	b/module.cc
# Module flags
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_LDFLAGS = \
	-avoid-version \
	-module \
	-shared \
	$(top_builddir)/v8-commonjs/libv8-commonjs.la
MODULES = $(module_LTLIBRARIES)
include $(top_srcdir)/automake/module.mk
