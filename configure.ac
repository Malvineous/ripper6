#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([ripper6], [0.1], [https://github.com/Malvineous/ripper6/issues], [], [http://github.com/Malvineous/ripper6])
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([libgamecommon], [libgamecommon])

BOOST_REQUIRE([1.46])
BOOST_TEST

AM_SILENT_RULES([yes])

AC_OUTPUT(Makefile src/Makefile)
