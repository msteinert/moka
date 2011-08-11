#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# library definition
lib_LTLIBRARIES = libv8-module.la
libv8_module_la_SOURCES = \
	module.cc
# include directories & compiler switches
libv8_module_la_CPPFLAGS = \
	-DPREFIX=\"$(prefix)\" \
	-I$(top_srcdir)
libv8_module_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
# install development headers
pkgincludedir = $(includedir)/v8
pkginclude_HEADERS = \
	module.h
