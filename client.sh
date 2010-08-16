#!/bin/sh

CLIENT_CMD="signalserver-sclient"

if [ ! -f bin/${CLIENT_CMD} ]; then
  echo "** ERROR - bin/${CLIENT_CMD} does not exist."
  exit 1
fi

exec env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib bin/${CLIENT_CMD} $*
