#! /bin/sh

SQL=sqlite3
SQLDIR=`KVCONFIG --datadir`/kvsynopd/
LOGDIR=`KVCONFIG --logdir`/kvsynop/
DBFILE=`KVCONFIG --localstatedir`/lib/kvsynopd/kvsynopd.sqlite

DAY=`date '+%d'`
LOG=$LOGDIR/kvsynopdb-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLDIR/cleansynopdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
