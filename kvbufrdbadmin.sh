#! /bin/sh

SQL=sqlite3

LOCAL_CONF_FILE=`KVCONFIG --sysconfdir`/kvalobs/kvbufrd_dbclean.sql
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

(test -f "$HOME/etc/KVALOBS_TEST") || ( /usr/local/sbin/alias_list | /bin/grep -q kvalobs ) || die


echo -n "Start: " > $LOG
date >> $LOG
echo "Clean script: $SQLCLEAN" >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLCLEAN >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
