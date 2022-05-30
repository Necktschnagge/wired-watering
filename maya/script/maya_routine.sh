#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

LOCAL_WORKING_BRANCH="productive-clone"

#BRANCH_TO_LOAD_AS_WORKING_BRANCH="productive"
BRANCH_TO_LOAD_AS_WORKING_BRANCH="nchain"
BRANCH_TO_LOAD_AS_FALLBACK_BRANCH="productive-fallback"

#current_sleep_time_s=900
current_sleep_time_s=900

run_counter=0



while true
do
	let run_counter++
	(
		echo "====================================================================================================="; \
		echo "====================================================================================================="; \
		echo "===     Starting with run #${run_counter}..."; \
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [1] Cleaning working directory..."; \
		echo "> sudo -u mayadm git status;"; \
		sudo -u mayadm git status; \
		echo "> sudo -u mayadm git add -u; git add *;"; \
		sudo -u mayadm git add -u; git add *; \
		echo "> sudo -u mayadm git status;"; \
		sudo -u mayadm git status; \
		echo "> sudo -u mayadm git ####...."
		#check if changes.
		#if so: new branch, commit, 
		echo "> sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --"
		sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --

		echo "> sudo -u mayadm git reset --hard;"; \
		sudo -u mayadm git reset --hard; \
		echo "Cleaning working directory   ...DONE!"; \
		echo "%%%%%%%%%%     [1] Cleaning working directory   ...DONE!"; \
		echo "====================================================================================================="; \
		sleep 5s
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [2] Loading source branch, pulling update from origin..."; \
		###
		echo "Loading and updating working branch..."; \
		sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}; \
		(
			sudo -u mayadm git pull && echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} successfully!"
		) || 
		(
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} failed! Resetting to last commit on local branch ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}" && git git reset --hard
		) \
	) | sudo -u mayadm tee logs/01.log
	echo "%%%%%%%%%%     [2] Loading source branch, pulling update from origin   ...DONE!"; \
	echo "====================================================================================================="; \
	sleep 10s
	echo "%%%%%%%%%%     [2] Loading source branch, pulling and switching to new branch for maya_routine..."; \
	echo "Switching to working branch..."
	sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}
	sleep 10s #let the servers connect to wifi!
	
	sudo -u mayadm git submodule update --init --recursive
	sudo -u mayadm ./up-unix-create-project.sh
	sudo -u mayadm ./ub-unix-build.sh
	sudo -u mayadm ./ur-unix-run.sh
	
	
	echo "Cleaning git working directory..."
	sudo -u mayadm git add -u; sudo -u mayadm git add *; \
	sudo -u mayadm git status; \
	sudo -u mayadm git reset --hard; \
	echo "Deleting local working branch..."
	sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
	sudo -u mayadm git branch -D ${LOCAL_WORKING_BRANCH}

	echo "Finished run ${run_counter}!"
	date
	echo "Going to sleep..."
	sleep 10s
	rtcwake -m mem -s ${current_sleep_time_s}
	echo "Waking up from sleep..."
	date
done
