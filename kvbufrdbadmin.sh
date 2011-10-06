#! /bin/sh

SQL=sqlite3

LOCAL_CONF_FILE=`KVCONFIG --sysconfdir`/kvalobs/kvbufrd_dbclean.sql
ETCDIR==`KVCONFIG --sysconfdir`/kvalobs
SQLDIR=`KVCONFIG --datadir`/kvbufrd
LOGDIR=`KVCONFIG --logdir`
DBFILE=`KVCONFIG --localstatedir`/lib/kvbufrd/kvbufr.sqlite
SQLCLEAN=$SQLDIR/cleanbufrdb.sql

if [ -f $LOCAL_CONF_FILE ]; then
	SQLCLEAN=$LOCAL_CONF_FILE
fi

DAY=`date '+%d'`
LOG=$LOGDIR/kvbufr/kvbufrdbadmin-$DAY.log

die()
{
   tstamp=`date`
   echo "$tstamp - Do NOT clean db on this machine." > $LOG
   exit 0
}

has_alias()
{
	aliasname=$1
	if [ -f /usr/local/sbin/alias_list ]; then
		/usr/local/sbin/alias_list | /bin/grep -q $aliasname
		return $?
	fi
	return 1
}

#(test -f "$ETCDIR/KVALOBS_TEST") || ( /usr/local/sbin/alias_list | /bin/grep -q kvalobs ) || die
(test -f "$ETCDIR/KVALOBS_TEST") || has_alias kvalobs || die


echo -n "Start: " > $LOG
date >> $LOG
echo "Clean script: $SQLCLEAN" >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLCLEAN >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
