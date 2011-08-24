#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

check_PROGRAMS = v8-commonjs

v8_commonjs_SOURCES = v8-commonjs.cc
v8_commonjs_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
v8_commonjs_LDFLAGS = $(top_builddir)/v8-commonjs/libv8-commonjs.la -lv8
