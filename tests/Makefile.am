#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

check_LTLIBRARIES = foo.la

foo_la_SOURCES = foo.cc

foo_la_CPPFLAGS = -I$(top_srcdir)

foo_la_LDFLAGS = -avoid-version -module -shared -export-dynamic -rpath /nowhere

check_PROGRAMS = v8-module

v8_module_SOURCES = v8-module.cc

v8_module_CPPFLAGS = -I$(top_srcdir)

v8_module_LDFLAGS = $(top_builddir)/v8-commonjs/libv8-commonjs.la -lv8
