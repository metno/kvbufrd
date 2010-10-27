#! /bin/sh

SQL=sqlite3

LOCAL_CONF_FILE=`KVCONFIG --sysconfdir`/kvalobs/bufrdbclean.sql
SQLDIR=`KVCONFIG --datadir`/kvbufrd/
LOGDIR=`KVCONFIG --logdir`
DBFILE=`KVCONFIG --localstatedir`/lib/kvbufrd/kvbufr.sqlite

if [ -f $LOCAL_CONF_FILE ]; then
	DBFILE=$LOCAL_CONF_FILE
fi

DAY=`date '+%d'`
LOG=$LOGDIR/kvbufr/kvbufrdbadmin-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLDIR/cleanbufrdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
