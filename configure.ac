#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_PREREQ([2.64])
# Library versioning
# Making releases:
# 	micro_version += 1
# 	interface_age += 1
# If functions have been added or backward-compatibility has been broken:
# 	interface_age = 0
m4_define([major_version], [0])
m4_define([minor_version], [1])
m4_define([micro_version], [0])
m4_define([interface_age], [0])
# Initialize autoconf & automake
AC_INIT([V8-CommonJS], [major_version.minor_version.micro_version],
	[http://github.com/msteinert/v8-commonjs/issues/], [v8-commonjs],
	[http://msteinert.github.com/v8-commonjs/])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([v8-commonjs/commonjs.h])
# Initialize Libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])
# Checks for programs
AC_PROG_CXX
AC_LANG([C++])
PKG_PROG_PKG_CONFIG([0.17])
# 32-Bit build
AC_ARG_ENABLE([32-bit],
	[AS_HELP_STRING([--enable-32-bit],
		[enable a 32-bit build @<:@default=no@:>@])],,
	[enable_32_bit=no])
AS_IF([test "x$enable_32_bit" = "xyes"],
	[AX_TRY_CXXFLAGS([-m32],
		[AX_CXXFLAGS([-m32])],
		[AC_MSG_ERROR([Failed to enable 32-bit build])])])
# Enable compiler flags
AX_TRY_CXXFLAGS([-Wall], [AX_CXXFLAGS([-Wall])])
AX_TRY_CXXFLAGS([-Wextra], [AX_CXXFLAGS([-Wextra])])
AX_TRY_CXXFLAGS([-Werror], [AX_CXXFLAGS([-Werror])])
AX_TRY_CXXFLAGS([-Wnon-virtual-dtor], [AX_CXXFLAGS([-Wnon-virtual-dtor])])
AX_TRY_CXXFLAGS([-Wno-unused-parameter], [AX_CXXFLAGS([-Wno-unused-parameter])])
AX_TRY_CXXFLAGS([-g], [AX_CXXFLAGS([-g])])
AX_TRY_CXXFLAGS([-ansi], [AX_CXXFLAGS([-ansi])])
AX_TRY_CXXFLAGS([-pedantic], [AX_CXXFLAGS([-pedantic])])
AX_TRY_CXXFLAGS([-fomit-frame-pointer], [AX_CXXFLAGS([-fomit-frame-pointer])])
AX_TRY_CXXFLAGS([-fdata-sections], [AX_CXXFLAGS([-fdata-sections])])
AX_TRY_CXXFLAGS([-ffunction-sections], [AX_CXXFLAGS([-ffunction-sections])])
AX_TRY_CXXFLAGS([-fno-rtti], [AX_CXXFLAGS([-fno-rtti])])
AX_TRY_CXXFLAGS([-fno-exceptions], [AX_CXXFLAGS([-fno-exceptions])])
AX_TRY_CXXFLAGS([-fvisibility=hidden], [AX_CXXFLAGS([-fvisibility=hidden])])
# Libtool versioning
LT_CURRENT=m4_eval(100 * minor_version + micro_version - interface_age)
LT_REVISION=interface_age
LT_AGE=m4_eval(m4_eval(100 * minor_version + micro_version) - interface_age)
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])
# Configure module directory
moduledir="\$(libdir)/v8-commonjs"
AC_SUBST([moduledir])
# Check for V8
AC_CHECK_HEADERS([v8.h],,
	[AC_MSG_ERROR([V8 is required to compile $PACKAGE_NAME])])
# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	tests/Makefile
	v8-commonjs/Makefile
])
AC_OUTPUT
AC_MSG_RESULT([
$PACKAGE_NAME build options:
	Host: $host
	Version: $VERSION
	32-Bit: $enable_32_bit
])
