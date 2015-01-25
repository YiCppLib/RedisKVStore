#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Perlude
AC_PREREQ([2.69])
AC_INIT([YiCppLib::RedisKVStore], [1.0.0], [atrusqiao@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AR
LT_PREREQ([2.2])
LT_INIT()
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

AC_CONFIG_SRCDIR([src/RedisKVStore.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
				 vendor/Makefile
                 src/Makefile])

AC_OUTPUT
