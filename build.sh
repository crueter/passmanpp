#!/bin/sh
function checklib() {
	if [ "`ldconfig -p | grep -i $1`" == "" ]
	then
		echo "Please install the $TO library (check your distribution's repositories)."
		exit 1
	fi
	return 0
}

checklib "botan" "botan"
checklib "qt6" "Qt 6"
cmake src -Wno-dev
make || exit 1
if [ "$1" == "clean" ]
then
	make clean
fi
