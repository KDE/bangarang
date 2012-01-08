#!/bin/bash 
$EXTRACTRC app/ui/*.ui >> rc.cpp
$XGETTEXT *.cpp app/*.cpp app/common/*.cpp app/medialists/*.cpp app/nowplaying/*.cpp platform/*.cpp platform/infofetchers/*.cpp platform/listengines/*.cpp platform/utilities/*.cpp -o ../po/bangarang.pot
rm -f rc.cpp
