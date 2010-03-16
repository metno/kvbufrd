#!/bin/sh
# prerm script for kvalobs
#
# see: dh_installdeb(1)

set -e

# summary of how this script can be called:
#        * <prerm> `remove'
#        * <old-prerm> `upgrade' <new-version>
#        * <new-prerm> `failed-upgrade' <old-version>
#        * <conflictor's-prerm> `remove' `in-favour' <package> <new-version>
#        * <deconfigured's-prerm> `deconfigure' `in-favour'
#          <package-being-installed> <version> `removing'
#          <conflicting-package> <version>
# for details, see http://www.debian.org/doc/debian-policy/ or
# the debian-policy package


case "$1" in
    remove|upgrade|deconfigure)
#    if which kvstop >/dev/null 2>&1; then
#       echo "preinst: Stopping kvalobs"
#       su --login kvalobs -c kvstop || true
#       echo "preinst: Stopped kvalobs"
#    fi
    if which invoke-rc.d >/dev/null 2>&1; then
       echo "prerm: Stopping kvalobs."
       invoke-rc.d kvalobs stop
       echo "prerm: kvalobs stopped."
    else
       echo "prerm: Stopping kvalobs."
       /etc/init.d/kvalobs stop
       echo "prerm: kvalobs stopped."
    fi
    ;;

    failed-upgrade)
    ;;

    *)
        echo "prerm called with unknown argument \`$1'" >&2
        exit 1
    ;;
esac

# dh_installdeb will replace this with shell code automatically
# generated by other debhelper scripts.

#DEBHELPER#

exit 0

