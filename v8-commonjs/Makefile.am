#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

SUBDIRS = . binary

# V8-CommonJS library
lib_LTLIBRARIES = libv8-commonjs.la
libv8_commonjs_la_SOURCES = \
	commonjs.cc \
	iconv.cc \
	iconv.h \
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
	-I$(top_builddir) \
	-DLIBDIR=\"$(libdir)\"
libv8_commonjs_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-ldl
# Modules
moduledir = $(libdir)/v8-commonjs
module_LTLIBRARIES = \
	assert.la \
	system.la \
	test.la
assert_la_SOURCES = \
	assert.cc \
	assert.h
system_la_SOURCES = \
	system.cc \
	system.h
test_la_SOURCES = \
	test.cc
# Module flags
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_LDFLAGS = \
	-avoid-version \
	-module \
	-shared \
	libv8-commonjs.la
MODULES = $(module_LTLIBRARIES)
include $(top_srcdir)/automake/module.mk
# Module dependencies
$(module_LTLIBRARIES): libv8-commonjs.la
# install development headers
pkgincludedir = $(includedir)/v8-commonjs
pkginclude_HEADERS = \
	commonjs.h \
	macros.h \
	module.h \
	module-loader.h
