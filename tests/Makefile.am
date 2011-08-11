#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

check_PROGRAMS = v8-module

v8_module_SOURCES = \
	v8-module.cc

v8_module_CPPFLAGS = \
	-I$(top_srcdir)

v8_module_LDFLAGS = \
	$(top_builddir)/v8/libv8-module.la -lv8
