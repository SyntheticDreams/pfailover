#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([pfailover], [0.4.1], [twestbrook@synthdreams.com])
AC_CONFIG_SRCDIR([src/conf.cpp])
#AC_CONFIG_HEADERS([config.h])

# Set flags to nothing so we don't generate debug info
CXXFLAGS=""
CFLAGS=""

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK

# Autoconf
AM_INIT_AUTOMAKE([pfailover], [0.4.1])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 data/Makefile])
AC_OUTPUT
