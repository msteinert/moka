#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# V8-CommonJS library
lib_LTLIBRARIES = libv8-commonjs.la
libv8_commonjs_la_SOURCES = \
	module.cc \
	module-factory.cc \
	module-factory.h \
	module-loader.cc \
	script-module.cc \
	script-module.h \
	so-module.cc \
	so-module.h
libv8_commonjs_la_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLIBDIR=\"$(libdir)\"
libv8_commonjs_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-ldl
# Modules
module_LTLIBRARIES = \
	binary.la \
	system.la
binary_la_SOURCES = binary.cc
system_la_SOURCES = system.cc
# Module flags
AM_CPPFLAGS = -I$(top_srcdir)
AM_LDFLAGS = \
	-avoid-version \
	-module \
	-shared \
	libv8-commonjs.la
# Module dependencies
$(module_LTLIBRARIES): libv8-commonjs.la
# install development headers
pkgincludedir = $(includedir)/v8-commonjs
pkginclude_HEADERS = \
	binary.h \
	commonjs.h \
	macros.h \
	module.h \
	module-loader.h \
	system.h
