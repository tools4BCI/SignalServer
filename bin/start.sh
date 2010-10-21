#!/bin/sh

CLIENT_CMD="./signalserver"

if [ ! -f ${CLIENT_CMD} ]; then
  echo "** ERROR - ${CLIENT_CMD} does not exist."
  exit 1
fi

exec env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../lib ${CLIENT_CMD}
