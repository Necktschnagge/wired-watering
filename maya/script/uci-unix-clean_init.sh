#!/bin/bash
cd ..
echo "Cleaning all files   (git clean -f -d -X)"
git clean -f -d -X
echo "Init git submodules"
git submodule update --init --recursive
cd script