AC_PREREQ([2.59])
AC_INIT([kvbufrd], [2.5.10~rc7], [kvoss@met.no])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL
#LT_INIT([dlopen])
AM_PROG_LEX
AM_PROG_CC_C_O
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_HEADERS(config.h)
PKG_PROG_PKG_CONFIG

AC_SUBST([BUFRTABLES],[`pwd`/src/bufrtables])

KV_FIND_OMNIORB4
PKG_CHECK_MODULES(putools, puTools)
LIBKVCPP2
PKG_CHECK_MODULES(sqlite3, sqlite3)


#GTEST_CHECK
#GMOCK_CHECK
AC_PATH_PROG(CMAKE, cmake, false)
GMOCK_DIST_CHECK([src/gtest])

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
                 share/bufrtables/Makefile
                 src/gtest/Makefile
                 check_kvbufrd.sh
                 src/kvbufrd_initdb
                 src/bufr/Makefile
                 src/Makefile
])




AC_OUTPUT
