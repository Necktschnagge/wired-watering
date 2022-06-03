#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi


LOCAL_WORKING_BRANCH="routine-standard-branch-739"
BRANCH_TO_LOAD_AS_WORKING_BRANCH="maya-productive"
BRANCH_TO_LOAD_AS_FALLBACK_BRANCH="maya-fallback"

current_sleep_time_s=900		#standard: 900 (15min) 
debug_step_sleep_s=15			#productive: 0 (no sleeps), debug: 15 (make log readable while running)

logs_path="../../../logs" ##### take a dir outside the repository (second clone to update logs to artifacts branch....)
run_counter_file_path="../artifacts/counter.info"

run_counter=0

if [[ -f "$run_counter_file_path" ]]; then
	echo "Found ${run_counter_file_path} from previous run to load run number."
	typeset -i run_counter=$(cat ${run_counter_file_path})
fi


while true
do
	let run_counter++
	(
		echo "====================================================================================================="; \
		echo "====================================================================================================="; \
		echo "===     Starting with run #${run_counter}..."; \
		echo "====================================================================================================="; \
		echo "====================================================================================================="; \
		echo "${run_counter}" | sudo -u mayadm tee ${run_counter_file_path}; \
		date; \
		echo "====================================================================================================="; \
		sleep ${debug_step_sleep_s}s; \
		echo "%%%%%%%%%%     [1] Cleaning working directory..."; \
		echo "> sudo -u mayadm git status;"; \
				sudo -u mayadm git status; \
		echo "> sudo -u mayadm git add -u; sudo -u mayadm git add *;"; \
				sudo -u mayadm git add -u; sudo -u mayadm git add *; \
		echo "> sudo -u mayadm git status;"; \
				sudo -u mayadm git status; \
		sleep ${debug_step_sleep_s}s; \
		# check if uncommitted changes are there and would be discarded if not handling it...
		#echo "> sudo -u mayadm git ####...."
		echo "Checking if there are uncommitted changes..."; \
		(
			echo "> sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --"; \
					sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --
		) || (
			echo "********** WARNING **********"; \
			echo "Found uncommitted changes, staged it, will commit it to a new branch..."; \
			timestamp=$(date +%s); \
			commit=$(sudo -u mayadm git rev-parse HEAD | cut -c0-7); \
			echo "> sudo -u mayadm git switch -c \"dev-${commit}-${timetamp}\""; \
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
		echo "%%%%%%%%%%     [2] Loading source branch, pulling updates from origin..."; \
		#echo "Loading and updating working branch..."; \
		echo "> sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"; \
				sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}; \
		(
			echo "> sudo -u mayadm git pull"; \
					sudo -u mayadm git pull && \
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} successfully!"
		) || \
		(
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} failed!"
			echo "Will build last commit on local branch ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"; \
			echo "> sudo -u mayadm git reset --hard"; \
					sudo -u mayadm git reset --hard
		); \
		commit=$(sudo -u mayadm git rev-parse HEAD); \
		echo "Last commit is:   ${commit}"
		echo "%%%%%%%%%%     [2] Loading source branch, pulling updates from origin   ...DONE!"; \
		echo "====================================================================================================="; \
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-2.log
	
	build_success=true
	sleep ${debug_step_sleep_s}s
	(
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [3] Switching to new branch and building sources..."; \
		echo
		echo
		echo "%%%%%%%%%%     [3.1] Switching to new branch and updating git submodules..."; \

		echo "Determining working branch name..."
		commit_short=$(sudo -u mayadm git rev-parse HEAD | cut -c1-8); \
		seconds_since_epoch=$(date -u +%s); \
		LOCAL_WORKING_BRANCH="routine-${commit_short}-${seconds_since_epoch}"; \
		echo "branch name:   ${LOCAL_WORKING_BRANCH}"; \
		
		echo "Switching to working branch..."; \
		echo "> sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}"; \
				sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}
		
		echo "Updating git submodules..."
		(
			echo "> timeout 240m sudo -u mayadm git submodule update --init --recursive"; \
					timeout 240m sudo -u mayadm git submodule update --init --recursive
		) && echo "successfully updated git submodules" || echo "git submodul update FAILED!"
		
		(
			echo "%%%%%%%%%%     [3.2] Creating project and building sources (incremental build)..."; \
			(
				echo "> timeout 60m sudo -u mayadm ./up-unix-create-project.sh"; \
						timeout 60m sudo -u mayadm ./up-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh"; \
						timeout 60m sudo -u mayadm ./ub-unix-build.sh
			) && (
				echo "Build successfully finished! (incremental build)"
				echo "%%%%%%%%%%     [3.3] Skipped (clean build)"; \
				echo "%%%%%%%%%%     [3.4] Skipped (fallback clean build)"; \
			)
		) ||
		
		(
			echo "%%%%%%%%%%     [3.3] Creating project and building sources (clean build)..."; \
			(
				echo "> timeout 60m sudo -u mayadm ./uc-unix-clean.sh"; \
						timeout 60m sudo -u mayadm ./uc-unix-clean.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./up-unix-create-project.sh"; \
						timeout 60m sudo -u mayadm ./up-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh"; \
						timeout 60m sudo -u mayadm ./ub-unix-build.sh
			) && (
				echo "Build successfully finished! (clean build)"
				echo "%%%%%%%%%%     [3.4] Skipped (fallback clean build)"; \
			)
		) ||
		
		(
			echo "%%%%%%%%%%     [3.4] Creating project and building sources (fallback clean build)..."; \
			(
				echo "> sudo -u mayadm git add * && sudo -u mayadm git add -u && sudo -u mayadm git fetch origin && sudo -u mayadm git reset --hard origin/${BRANCH_TO_LOAD_AS_FALLBACK_BRANCH}"; \
						sudo -u mayadm git add * && sudo -u mayadm git add -u && sudo -u mayadm git fetch origin && sudo -u mayadm git reset --hard origin/${BRANCH_TO_LOAD_AS_FALLBACK_BRANCH}
			) && (
				echo "> timeout 60m sudo -u mayadm ./uc-unix-clean.sh"; \
						timeout 60m sudo -u mayadm ./uc-unix-clean.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./up-unix-create-project.sh"; \
						timeout 60m sudo -u mayadm ./up-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh"; \
						timeout 60m sudo -u mayadm ./ub-unix-build.sh
			) && (
				echo "Build successfully finished! (fallback clean build)"
			)
		) ||
		
		(
			echo "**********************************************************"
			echo "************                                  ************"
			echo "************   FATAL! ALL BUILDS FAILED !!!   ************"
			echo "************                                  ************"
			echo "**********************************************************"
			build_success=false
		)
		echo "%%%%%%%%%%     [3] Switching to new branch and building sources   ...DONE!"; \
		echo "====================================================================================================="; \
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-3.log
	
	
	
	(
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [4] Run the executable..."; \
		
		sleep 10s ######let the servers connect to wifi! ## remove this, c++ should check this via ping and wait.
		
		if [ "$build_success" = true ] ; then
			#######add timeout here when sleep mode implies valve shutdown.
			echo "> sudo -u mayadm ./ur-unix-run.sh"; \
					sudo -u mayadm ./ur-unix-run.sh 
			########use timeout XXXs ./run # 4 hours or so...
		else
			echo "FATAL! BUILD FAILED IN PREVIOUS STEP. WILL NOT EXECUTE!"
		fi
		echo "%%%%%%%%%%     [4] Run the executable   ...DONE!"; \
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-4.log
	

	(
		echo "====================================================================================================="; \
		echo "%%%%%%%%%%     [5] Cleaning git working directory..."; \
		####### enhance this step 5!!!
		sudo -u mayadm git add -u; \
		sudo -u mayadm git add *; \
		sudo -u mayadm git status; \
		sudo -u mayadm git reset --hard; \
		echo "Deleting local working branch..."
		sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
		sudo -u mayadm git branch -D ${LOCAL_WORKING_BRANCH}

		echo "Finished run ${run_counter}!"
		date
		echo "%%%%%%%%%%     [5] Cleaning git working directory   ...DONE!"; \
		echo "====================================================================================================="; \
	) 2>&1 | sudo -u mayadm tee ${logs_path}/${run_counter}-5.log
	
	(
		#######'Uploading logs!'
		sudo -u mayadm git checkout artifacts
		cp ${logs_path} ../artifacts/logs --recursive
		sudo -u mayadm git add -u
		sudo -u mayadm git add *
		sudo -u mayadm git commit -m "upload logs" && git push

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
