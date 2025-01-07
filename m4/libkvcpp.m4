#
# SYNOPSIS
#
#   LIBKVCPP
#
# DESCRIPTION
#
#   Checks for libkvcpp, using pkg-config. Also define KVIDLDIR. KVIDLDIR is the
#   path to the CORBA IDL files that defines the interface to kvalobs.
#
#   This macro calls:
#
#     AC_SUBST(KVIDLDIR)
#
# LAST MODIFICATION
#
#   2010-04-16
#
# COPYLEFT
#
#   Copyright (c) 2007 Meteorologisk institutt <kvoss@met.no>
#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see
#   <http://www.gnu.org/licenses/>.
#




AC_DEFUN([LIBKVCPP],
[
AC_ARG_WITH(
    [kvalobs],
    AS_HELP_STRING([--with-kvalobs=PATH]
                   [Specify which kvalobs to build against.]),
    [KVBUILD_DIR="${with_kvalobs}"],
    [KVBUILD_DIR=""]
)

AC_ARG_WITH(
    [kvalobs-static],
    AS_HELP_STRING([--with-kvalobs-static]
                   [Link static against kvalobs library.]),
    [KVSTATIC_LIBS="true"],
    [KVSTATIC_LIBS="false"]
)

AC_MSG_RESULT([KVBUILD_DIR: ${KVBUILD_DIR}])
if test -z "${KVBUILD_DIR}"; then
    PKG_CHECK_MODULES(kvsubscribe, libkvsubscribe)
    KVLIBDIR=`${PKG_CONFIG} --variable=libdir libkvsubscribe`
    KVINCLUDEDIR=`${PKG_CONFIG} --variable=includedir libkvsubscribe`
    KVCFLAGS=`${PKG_CONFIG} --cflags-only-other libkvsubscribe`
    KVLDFLAGS=`${PKG_CONFIG} --libs-only-other libkvsubscribe`
else
    PKGCONF_SAVED=${PKG_CONFIG_PATH}
    export PKG_CONFIG_PATH="${KVBUILD_DIR}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    PKG_CHECK_MODULES(kvsubscribe, libkvsubscribe)
    
    KVLIBDIR=${KVBUILD_DIR}/lib
    KVINCLUDEDIR=${KVBUILD_DIR}/include
    KVCFLAGS=`${PKG_CONFIG} --cflags-only-other libkvsubscribe`
    KVLDFLAGS=`${PKG_CONFIG} --libs-only-other libkvsubscribe`
    KVIDLDIR=${KVBUILD_DIR}/share/kvalobs/idl
    AC_MSG_RESULT([KVLIBDIR: ${KVBUILD_DIR}/lib])
    AC_MSG_RESULT([KVINCLUDEDIR: ${KVBUILD_DIR}/lib])
    AC_MSG_RESULT([KVCFLAGS: ${KVCFLAGS}])
    AC_MSG_RESULT([KVLDLAGS: ${KVLDLAGS}])
    AC_MSG_RESULT([PKG_CONFIG_PATH: ${PKG_CONFIG_PATH}])
    AC_MSG_RESULT([KVLIBDIR: ${KVBUILD_DIR}/lib])
    export PKG_CONFIG_PATH=${PKGCONF_SAVED}
fi

KVLIBPREFIX=libkvalobs

AC_SUBST(KVLIBPREFIX)
AC_SUBST(KVIDLDIR)
AC_SUBST(KVLIBDIR)
AC_SUBST(KVINCLUDEDIR)
AC_SUBST(KVCFLAGS)
AC_SUBST(KVLDFLAGS)


#AM_CONDITIONAL([KVSTATIC], test "${KVBUILD_DIR}" )

#Allways build static
AM_CONDITIONAL([KVSTATIC], test "${KVSTATIC_LIBS}" = "true" )
])
