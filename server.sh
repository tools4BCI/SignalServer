#!/bin/bash

CMD="signalserver"

if [ ! -f bin/${CMD} ]; then
  echo "** ERROR - bin/${CMD} does not exist."
  exit 1
fi

PLATFORM=$(uname -m)

if [ "$PLATFORM" == "x86_64" ]
then
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/amd64:$(pwd)/extern/lib/tobicore/linux/amd64:$(pwd)/extern/lib/tobiid/linux/amd64:$(pwd)/extern/lib/libtid/linux/amd64:$(pwd)/extern/lib/tobiic/linux/amd64
else
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/extern/lib/tia/linux/x86:$(pwd)/extern/lib/tobicore/linux/x86:$(pwd)/extern/lib/tobiid/linux/x86:$(pwd)/extern/lib/libtid/linux/x86:$(pwd)/extern/lib/tobiic/linux/x86
fi

export PATH LD_LIBRARY_PATH

exec ./bin/${CMD}  $1 $2
#exec valgrind  ./bin/${CMD}  $1 $2
#exec valgrind  --leak-check=full --show-reachable=yes ./bin/${CMD}  $1 $2

