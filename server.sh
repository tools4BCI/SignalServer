#!/bin/bash

CMD="signalserver"

if [ ! -f bin/${CMD} ]; then
  echo "** ERROR - bin/${CMD} does not exist."
  exit 1
fi

PLATFORM=$(uname -m)

if [ "$PLATFORM" == "x86_64" ]
then
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/x64
else
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/x32
fi

export PATH LD_LIBRARY_PATH

exec ./bin/${CMD}  $1 $2
# exec valgrind  ./bin/${CMD}  $1 $2
# exec valgrind  --leak-check=full --show-reachable=yes ./bin/${CMD}  $1 $2

