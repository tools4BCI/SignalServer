#!/bin/bash

CMD="test"

if [ ! -f bin/${CMD} ]; then
  echo "** ERROR - bin/${CMD} does not exist."
  exit 1
fi

PLATFORM=$(uname -m)

if [ "$PLATFORM" == "x86_64" ]
then
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/amd64
else
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/x86
fi

export PATH LD_LIBRARY_PATH
exec ./bin/${CMD}  $1 $2


