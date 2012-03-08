#!/bin/bash 
$EXTRACTRC $(find -name "*.ui") >> rc.cpp
$XGETTEXT $(find -name "*.cpp") -o $podir/bangarang.pot
rm -f rc.cpp
