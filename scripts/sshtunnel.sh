#!/bin/sh
APPDIR=/mnt/mmcblk0p2/App
REMOTEPORT=22001
REMOTEHOST=$1
LOCALPORT=22

if [ x$REMOTEHOST = x ] ; then
	REMOTEHOST=central.metalu.net
fi

#killall autossh
pkill -f "autossh .* -NR $REMOTEPORT"

screen -S TUNNEL -d -m $APPDIR/autossh/autossh -f -M 21990 -NR $REMOTEPORT:localhost:$LOCALPORT portail@$REMOTEHOST

