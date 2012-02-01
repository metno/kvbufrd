#! /bin/sh


has_alias()
{
	aliasname=$1
	if [ -f /usr/local/sbin/alias_list ]; then
		/usr/local/sbin/alias_list | /bin/grep -q $aliasname
		return $?
	fi
	return 1
}

if has_alias $1 ; then
	echo "Has alias $1"
else
	echo "Has NOT alias $1"
fi
