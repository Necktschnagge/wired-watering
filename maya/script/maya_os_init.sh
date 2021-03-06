#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

echo "Installing git..."
apt-get install -y git

echo "Configuring git..."
git config --global credential.helper store
git config --global user.name "Maya Server"
git config --global user.email "maya.watering@gmail.com"
git config --global --add safe.directory '*'

echo "Installing CMake..."
apt-get install -y cmake

echo "Installing g++..."
apt-get install -y g++

echo "Installing OpenSSL-dev"
apt-get install -y libssl-dev

exit #will exit with status of last command
