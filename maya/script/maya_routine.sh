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
debug_step_sleep_s=15

logs_path="logs"

run_counter=0



while true
do
	let run_counter++
	(
		echo "====================================================================================================="; \
		echo "====================================================================================================="; \
		echo "===     Starting with run #${run_counter}..."; \
		echo "====================================================================================================="; \
		echo "====================================================================================================="; \
		date; \
		echo "====================================================================================================="; \
		sleep ${debug_step_sleep_s}s; \
		echo "%%%%%%%%%%     [1] Cleaning working directory..."; \
		echo "> sudo -u mayadm git status;"; \
		sudo -u mayadm git status; \
		echo "> sudo -u mayadm git add -u; git add *;"; \
		sudo -u mayadm git add -u; git add *; \
		echo "> sudo -u mayadm git status;"; \
		sudo -u mayadm git status; \
		echo "> sudo -u mayadm git ####...."
		sleep ${debug_step_sleep_s}s; \
		# check if uncommitted changes are there and would be discarded if not handling it...
		echo "Checking if uncommitted changes are there..."; \
		echo "> sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --"; \
		(
		sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --
		) || (
			echo "********** WARNING **********"; \
			echo "Found uncommitted changes, staged it, will commit it to a new branch..."; \
			timestamp=$(date +%s); \
			commit=$(sudo -u mayadm git rev-parse HEAD | cut -c0-7); \
			echo '> sudo -u mayadm git switch -c "dev-${commit}-${timetamp}";' \
			sudo -u mayadm git switch -c "dev-${commit}-${timetamp}"; \
			echo "branch name is dev-${commit}-${timetamp}"; \
			echo '> sudo -u mayadm git commit -m "local changes automatically committed by maya_routine.sh"'; \
			sudo -u mayadm git commit -m "local changes automatically committed by maya_routine.sh"; \
			sleep ${debug_step_sleep_s}s; \
		)

		echo "> sudo -u mayadm git reset --hard;"; \
		sudo -u mayadm git reset --hard; \
		echo "%%%%%%%%%%     [1] Cleaning working directory   ...DONE!"; \
		echo "====================================================================================================="; \
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-1.log
	sleep ${debug_step_sleep_s}s; \
	(
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
		echo "%%%%%%%%%%     [2] Loading source branch, pulling update from origin   ...DONE!"; \
		echo "====================================================================================================="; \
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-2.log
	sleep ${debug_step_sleep_s}s
	(
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [3] Switching to new branch and building sources..."; \
		echo "Switching to working branch..."
		sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}
		sleep 10s #let the servers connect to wifi!
		sudo -u mayadm git submodule update --init --recursive
		sudo -u mayadm ./up-unix-create-project.sh
		sudo -u mayadm ./ub-unix-build.sh
		echo "====================================================================================================="; \
		# fail -> clean build [3.2]
		# again fail -> clean build on fallback branch. [3.3]
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-3.log
	(
		
		#[4] execute
		sudo -u mayadm ./ur-unix-run.sh
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-4.log
	#[5]
	(
	echo "Cleaning git working directory..."
	sudo -u mayadm git add -u; sudo -u mayadm git add *; \
	sudo -u mayadm git status; \
	sudo -u mayadm git reset --hard; \
	echo "Deleting local working branch..."
	sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
	sudo -u mayadm git branch -D ${LOCAL_WORKING_BRANCH}

	echo "Finished run ${run_counter}!"
	date
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-5.log
	
	(
		#'Uploading logs!'

		echo "##############################################################"
		echo "###################                        ###################"
		echo "###################     GOING TO SLEEP     ###################"
		echo "###################                        ###################"
		echo "##############################################################"
		echo "#######                                                #######"
		echo "#######   Press CTRL + C NOW to abort script safely!   #######"
		echo "#######                                                #######"
		echo "##############################################################"
		sleep 10s
		echo "##############################################################"
		echo "#######                                                #######"
		echo "#######               NOW GOING TO SLEEP               #######"
		echo "#######                                                #######"
		echo "##############################################################"
		sleep 2s
		echo "> rtcwake -m mem -s ${current_sleep_time_s}"
		rtcwake -m mem -s ${current_sleep_time_s}
		echo "##############################################################"
		echo "#######                                                #######"
		echo "#######            just waked up from sleep            #######"
		echo "#######                                                #######"
		echo "##############################################################"
		sleep ${debug_step_sleep_s}s
	)
done
