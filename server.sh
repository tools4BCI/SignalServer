#!/bin/sh

SERVER_CMD="signalserver"

if [ ! -f bin/${SERVER_CMD} ]; then
  echo "** ERROR - bin/${SERVER_CMD} does not exist."
  exit 1
fi

cd bin

exec env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../lib ./${SERVER_CMD} 
