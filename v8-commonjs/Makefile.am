#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# library definition
lib_LTLIBRARIES = libv8-commonjs.la
libv8_commonjs_la_SOURCES = \
	internal/module.cc \
	internal/module.h \
	module.cc
# include directories & compiler switches
libv8_commonjs_la_CPPFLAGS = \
	-DLIBDIR=\"$(libdir)\" \
	-I$(top_srcdir)
libv8_commonjs_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
# install development headers
pkgincludedir = $(includedir)/v8-commonjs
pkginclude_HEADERS = \
	module.h
