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
    PKG_CHECK_MODULES(kvcpp, libkvcpp)

	KVIDLDIR=`pkg-config --variable=idldir libkvcpp`
	if test ! -x $KVIDLDIR; then
		AC_MSG_ERROR([Unable to locate the idl files to KVALOBS])
	fi
	
	AC_SUBST(KVIDLDIR)
])


AC_DEFUN([LIBKVCPP2],
[
AC_ARG_WITH(
    [kvalobs],
    AS_HELP_STRING([--with-kvalobs=PATH]
                   [Specify wich kvalobs to build against.]),
    [KVBUILD_DIR="${with_kvalobs}"],
    [KVBUILD_DIR=""]
)

AC_ARG_WITH(
    [kvalobs-libversion],
    AS_HELP_STRING([--with-kvalobs-libversion=version]
                   [Specify wich kvalobs libberary to build against.]),
    [KVLIBVERSJON="${with_kvalobs_libversion}"],
    [KVLIBVERSJON=""]
)


AC_MSG_RESULT([KVBUILD_DIR: A${KVBUILD_DIR}A])
if test -z "${KVBUILD_DIR}"; then
    PKG_CHECK_MODULES(kvcpp, libkvcpp${KVLIBVERSJON})
    KVLIBDIR=`${PKG_CONFIG} --variable=libdir libkvcpp${KVLIBVERSJON}`
    KVINCLUDEDIR=`${PKG_CONFIG} --variable=includedir libkvcpp${KVLIBVERSJON}`
    KVCFLAGS=`${PKG_CONFIG} --cflags-only-other libkvcpp${KVLIBVERSJON}`
    KVLDFLAGS=`${PKG_CONFIG} --libs-only-other libkvcpp${KVLIBVERSJON}`

    KVIDLDIR=`${PKG_CONFIG} --variable=idldir libkvcpp${KVLIBVERSJON}`
    if test ! -x $KVIDLDIR; then
        AC_MSG_ERROR([Unable to locate the idl files to KVALOBS])
    fi
else
    PKGCONF_SAVED=${PKG_CONFIG_PATH}
    export PKG_CONFIG_PATH="${KVBUILD_DIR}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    PKG_CHECK_MODULES(kvcpp, libkvcpp${KVLIBVERSJON})
    
    KVLIBDIR=${KVBUILD_DIR}/lib
    KVINCLUDEDIR=${KVBUILD_DIR}/include/kvalobs
    KVCFLAGS=`${PKG_CONFIG} --cflags-only-other libkvcpp${KVLIBVERSJON}`
    KVLDFLAGS=`${PKG_CONFIG} --libs-only-other libkvcpp${KVLIBVERSJON}`
    KVIDLDIR=${KVBUILD_DIR}/share/kvalobs/idl
    AC_MSG_RESULT([KVLIBDIR: ${KVBUILD_DIR}/lib])
    AC_MSG_RESULT([KVINCLUDEDIR: ${KVBUILD_DIR}/lib])
    AC_MSG_RESULT([KVCFLAGS: ${KVCFLAGS}])
    AC_MSG_RESULT([KVLDLAGS: ${KVLDLAGS}])
    AC_MSG_RESULT([PKGCONF: ${PKG_CONFIG_PATH}/lib])
    AC_MSG_RESULT([KVLIBDIR: ${KVBUILD_DIR}/lib])
    export PKG_CONFIG_PATH=${PKGCONF_SAVED}
fi

KVLIBPREFIX=libkvalobs${KVLIBVERSJON}

AC_SUBST(KVLIBPREFIX)
AC_SUBST(KVIDLDIR)
AC_SUBST(KVLIBDIR)
AC_SUBST(KVINCLUDEDIR)
AC_SUBST(KVCFLAGS)
AC_SUBST(KVLDFLAGS)


#AM_CONDITIONAL([KVSTATIC], test "${KVBUILD_DIR}" )

#Allways build static
AM_CONDITIONAL([KVSTATIC], true )
])

