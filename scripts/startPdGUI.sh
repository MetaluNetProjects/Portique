#!/bin/sh

APPDIR=/mnt/mmcblk0p2/App
MAIN_PATCH=$APPDIR/`cat $APPDIR/MAIN_PATCH.txt`
echo "main patch: $MAIN_PATCH"

sudo killall pd
#sleep 5
#tce-load -i unzip
#sudo prlimit --rtprio=99 /home/tc/src/pure-data-ant1r-git/bin/pd -nogui -open /home/tc/Pd/test.pd&

#sudo prlimit --rtprio=99 /home/tc/src/pure-data-ant1r-git/bin/pd -open $MAIN_PATCH

$APPDIR/Pd/src/bin/pd -noaudio -nrt -open $MAIN_PATCH

