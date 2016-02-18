#! /bin/sh


files=`ls -1 ~/bufr/*.bufr`

for file in $files; do
    bufrdump $file
    read dummy
    clear
done
