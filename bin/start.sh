#!/bin/sh

CLIENT_CMD="./signalserver"

if [ ! -f ${CLIENT_CMD} ]; then
  echo "** ERROR - bin/${CLIENT_CMD} does not exist."
  exit 1
fi

exec env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../extern/lib/libusb-1.0.8/libusb/.libs ${CLIENT_CMD}

