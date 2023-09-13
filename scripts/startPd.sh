#!/bin/sh

APPDIR=/mnt/mmcblk0p2/App
MAIN_PATCH=$APPDIR/`cat $APPDIR/MAIN_PATCH.txt`
echo "main patch: $MAIN_PATCH"

#exit

sudo killall pd
sudo killall runOSC.sh

sleep 5

if [ x$1 != xapponly ] ; then
	#sudo cp $APPDIR/firmware/rtlwifi/* /lib/firmware/rtlwifi
	#sudo modprobe -r rtl8xxxu
	#sudo modprobe rtl8xxxu

	# Wifi AP
	if true; then
		sudo ifconfig wlan0 192.168.5.1
		sudo hostapd -B $APPDIR/hostapd.conf
		sudo dnsmasq -l /tmp/dnsmasq.leases -C $APPDIR/dnsmasq.conf
	fi

	# Wifi client
	if false; then
		sudo wpa_supplicant -i wlan0 -c $APPDIR/wpa_supplicant.conf -B -D nl80211,wext
		sudo udhcpc -n -i wlan0 -x hostname:`hostname`
	#sudo dhcpcd -f $APPDIR/dhcpcd.conf
	fi

	#sudo ifconfig wlan0 192.168.0.10

	#sudo rm -rf /home/data
	#sudo ln -s $APPDIR/data /home/data
	#sudo chown data:staff /home/data

	#sudo hostname Derby2
	#sudo ifconfig eth0 192.168.1.200

	#amixer -c 1 cset numid=6 28,28

	sudo cp $APPDIR/Pd/src/bin/pdsend $APPDIR/Pd/src/bin/pdreceive /usr/local/bin

	#sleep 5
	#killall autossh
	#$APPDIR/autossh/autossh -f -M 21990 -NR 22001:localhost:22 portail@central.metalu.net
	sshtunnel.sh central.metalu.net
fi


screen -S OSC -d -m /mnt/mmcblk0p2/App/Portique/OSCremote/runOSC.sh

sleep 5
#screen -S PD -d -m sudo prlimit --rtprio=99 $APPDIR/Pd/src/bin/pd -nogui -open $MAIN_PATCH

screen -S PD -d -m $APPDIR/Pd/src/bin/pd -noaudio -nrt -nogui -open $MAIN_PATCH



