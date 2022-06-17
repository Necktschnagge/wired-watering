#!/bin/bash

(timeout 60s dhclient usb0) && echo "Connecting to internet via usb0 tethering successful." || echo "Connecting to internet via usb0 tethering FAILED"

exit #will exit with status of last command
