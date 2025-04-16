AC_PREREQ([2.71])
AC_INIT([kvbufrd],[3.0.12],[kvoss@met.no])
AM_INIT_AUTOMAKE([foreign subdir-objects])
#AM_INIT_AUTOMAKE([foreign]) 
AC_CONFIG_MACRO_DIR([m4])

LT_INIT
#LT_INIT([dlopen])
#AM_PROG_LEX
#AC_PROG_YACC
AC_PROG_CXX
AC_PROG_INSTALL

#AX_CXX_COMPILE_STDCXX_11(noext)
AX_CXX_COMPILE_STDCXX([20],[noext],[mandatory])

AC_CONFIG_HEADERS(config.h)
PKG_PROG_PKG_CONFIG

AC_SUBST([BUFRTABLES],[`pwd`/src/bufrtables])

PKG_CHECK_MODULES(putools, puTools)
LIBKVCPP

#PKG_CHECK_MODULES(sqlite3, sqlite3)
AC_PATH_PROG(CMAKE, cmake, false)

#GMOCK_DIST_CHECK([src/gtest])
#GMOCK_DIST_CHECK([.])
GTEST_CHECK

AX_BOOST_BASE
AX_BOOST_THREAD
if test -z $BOOST_THREAD_LIB; then AC_MSG_ERROR([Unable to find boost thread library]); fi 
AX_BOOST_REGEX
if test -z $BOOST_REGEX_LIB; then AC_MSG_ERROR([Unable to find boost regex library]); fi 
AX_BOOST_FILESYSTEM
if test -z $BOOST_FILESYSTEM_LIB; then AC_MSG_ERROR([Unable to find boost filesystem library]); fi
AX_BOOST_PROGRAM_OPTIONS
if test -z $BOOST_PROGRAM_OPTIONS_LIB; then AC_MSG_ERROR([Unable to find boost program_options library]); fi
AX_BOOST_DATE_TIME
if test -z $BOOST_DATE_TIME_LIB; then AC_MSG_ERROR([Unable to find boost date_time library]); fi
AX_BOOST_SYSTEM
if test -z $BOOST_SYSTEM_LIB; then AC_MSG_ERROR([Unable to find boost system library]); fi

AC_CONFIG_FILES([Makefile
                 ecmwf/Makefile
                 debian_files/Makefile
                 share/bufrtables/Makefile
                 check_kvbufrd.sh
                 src/kvbufrd_initdb
                 src/bufr/Makefile
                 src/Makefile
                 src/kvbufrconf/Makefile
])

# src/gtest/Makefile



AC_OUTPUT
