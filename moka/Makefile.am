#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# Moka library
lib_LTLIBRARIES = libmoka.la
libmoka_la_SOURCES = \
	module.cc \
	module-factory.cc \
	module-factory.h \
	module-loader.cc \
	moka.cc \
	script-module.cc \
	script-module.h \
	so-module.cc \
	so-module.h
libmoka_la_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-DLIBDIR=\"$(libdir)\"
libmoka_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-ldl \
	-lv8
pkgincludedir = $(includedir)/moka
pkginclude_HEADERS = \
	macros.h \
	module.h \
	module-loader.h \
	moka.h
# Moka developer shell
bin_PROGRAMS = moka
moka_SOURCES = \
	moka.cc
moka_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir)
moka_LDFLAGS = \
	libmoka.la \
	-lv8
# Modules
moduledir = $(libdir)/moka
module_LTLIBRARIES = \
	lang.la
lang_la_SOURCES = \
	lang/throwable.cc \
	lang/throwable.h \
	lang/module.cc
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir)
AM_LDFLAGS = \
	-avoid-version \
	-module \
	-shared \
	libmoka.la
MODULES = $(module_LTLIBRARIES)
include $(top_srcdir)/automake/module.mk
$(module_LTLIBRARIES): libmoka.la
