#!/bin/sh
APPDIR=/mnt/mmcblk0p2/App
REMOTEPORT=28001
REMOTEHOST=$1
if [ x$REMOTEHOST = x ] ; then
	REMOTEHOST=central.metalu.net
fi
LOCALPORT=8080

#pkill -f "ssh -NR $REMOTEPORT"
pkill -f "autossh .* -NR $REMOTEPORT"

#screen -S OSC -d -m ssh -NR $REMOTEPORT:localhost:$LOCALPORT portail@$REMOTEHOST
$APPDIR/autossh/autossh -f -M 0 -NR $REMOTEPORT:localhost:$LOCALPORT portail@$REMOTEHOST

echo from your endpoint host, type:
echo ssh -o ProxyJump=portail@$REMOTEHOST -NL $REMOTEPORT:localhost:$REMOTEPORT portail@localhost
echo then open "http://localhost:$REMOTEPORT/" from your web browser.
