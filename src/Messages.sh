#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$XGETTEXT *.cpp -o $podir/choqok.pot
rm -f rc.cpp
