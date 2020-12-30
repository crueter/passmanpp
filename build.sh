#!/bin/sh
function checklib() {
	if [ "`ldconfig -p | grep -i $1`" == "" ]
	then
		if [ "$2" == "" ]
		then
			declare TO="$1"
		else
			declare TO="$2"
		fi
		echo "Please install the $TO library (check your distribution's repositories)."
		exit 1
	fi
	return 0
}

checklib "botan"
checklib "sodium"
checklib "qt6" "Qt 6"
cmake src -Wno-dev
make
if [ "$1" == "clean" ]
then
	make clean
fi
