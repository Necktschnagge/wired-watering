#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

#git init
git config --global credential.helper store
git config --global user.name "Maya Server"
git config --global user.email "maya.watering@gmail.com"

exit

#required software
apt-get install cmake

exit #will exit with status of last command
