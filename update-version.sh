
#!/bin/bash

echo "Checking version..."

# Don't update the version if nothing needs to be compiled
echo "Updating version."

numFiles="`make -n | wc -l`"

if [ $numFiles -lt 6 ]; then
    echo "Nothing to update. Project is up to date!"
    exit
fi


if [ -f include/version.h ]; then

  maj_ver="`sed  's/^ *//' major_version`"

  echo "#ifndef _VERSION_H
  #define _VERSION_H" > _version.h
  echo "  #define MAJOR_VERSION \"`sed  's/^ *//' major_version`\"" >> _version.h

  awk '{if ($2 ~ /BUILD_NUMBER/) print "  " $1 " " $2 " " ++$3;}' include/version.h >> _version.h

  echo "  #define BUILD_STR \" `date`\"" >> _version.h
  echo "#endif   //_VERSION_H"  >> _version.h

  mv _version.h include/version.h

else
  echo "#ifndef _VERSION_H
  #define _VERSION_H" > version.h
  echo "  #define MAJOR_VERSION \"`sed  's/^ *//' major_version`\"" >> version.h
  echo "  #define BUILD_NUMBER 1" >> version.h
  echo "  #define BUILD_STR \" `date`\"" >> version.h
  echo "#endif   //_VERSION_H"  >> version.h
  mv version.h include/version.h
fi

