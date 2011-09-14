#!/bin/bash 
# Script to extract translatable messages from the code.

# Work usually done by scripty in KDE svn
XGETTEXT="xgettext --copyright-holder=This_file_is_part_of_KDE --from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address=http://code.google.com/p/bangarangissuetracking/issues"
EXTRACTRC="extractrc"
export XGETTEXT EXTRACTRC

echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp
 
# Extract
echo 'Extracting messages...'
$EXTRACTRC app/ui/*.ui >> rc.cpp
$XGETTEXT *.cpp app/*.cpp app/common/*.cpp app/medialists/*.cpp app/nowplaying/*.cpp platform/*.cpp platform/infofetchers/*.cpp platform/listengines/*.cpp platform/utilities/*.cpp -o ../po/bangarang.pot

# Merge
echo 'Merging messages...'
for lang in $( find ../po  -maxdepth 1 -mindepth 1 -type d )
do
     msgmerge -U $lang/bangarang.po ../po/bangarang.pot 
done

# Clean
echo 'Cleaning up...'
rm -f rc.cpp

echo 'Done!'