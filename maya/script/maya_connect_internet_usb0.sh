#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

(timeout 60s dhclient usb0) && echo "Connecting to internet via usb0 tethering successful." || echo "Connecting to internet via usb0 tethering FAILED"

exit #will exit with status of last command
