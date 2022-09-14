#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

logs_path="../../../logs" ##### take a dir outside the repository (second clone to update logs to artifacts branch....)

while true
do

	cp ./maya_routine.sh ../../../logs/maya_routine.sh
	../../../logs/maya_routine.sh || (
		echo "> sudo -u mayadm git reset --hard"; \
				sudo -u mayadm git reset --hard; \
		echo "> sudo -u mayadm git clean -fd"; \
				sudo -u mayadm git clean -fd; \
		echo "> sudo -u mayadm git checkout maya-productive"; \
				sudo -u mayadm git checkout maya-productive; \
		echo "> sudo -u mayadm git pull"; \
				sudo -u mayadm git pull; \

	)

done
