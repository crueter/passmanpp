#!/bin/sh
if [ "$EUID" != "0" ]
then
	echo "Please run with sudo or as root."
fi

./build.sh
install -Dm755 passman /usr/local/bin/passman
