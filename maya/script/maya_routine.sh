#!/bin/bash
#
#
#  TODOS
#  files for commit: shell script, run_counter file, 6 log of prev run and 1..5 of current run. (?)
#			-->>  check why uploading to artifacts branch failed
#
#
# try to remove ; \ where not needed


############################################### check root ############################################################

if [ "$(id -u)" -ne 0 ]; then
  echo >&2 "Error: script not running as root or with sudo! Exiting..."
  exit 1
fi

############################################### config variables ############################################################

LOCAL_WORKING_BRANCH="routine-standard-branch-739"
BRANCH_TO_LOAD_AS_WORKING_BRANCH="maya-productive"
BRANCH_TO_LOAD_AS_FALLBACK_BRANCH="maya-fallback"

current_sleep_time_s=2400		#standard: 900 (15min), 1200 (20min), 1800 (30min), 2400 (40min), 3000 (50min)
debug_step_sleep_s=0			#productive: 0 (no sleeps), debug: 15 (make log readable while running)

logs_path="../../../logs" ##### take a dir outside the repository (second clone to update logs to artifacts branch....)
#logs_path="../../../logs/maya/artifacts/logs/" ##### take a dir outside the repository (second clone to update logs to artifacts branch....)
user_home="/home/mayadm"
run_counter_file_path="${user_home}/repos/logs/counter.info"

run_counter=0

############################################### modify variables ############################################################

if [[ -f "$run_counter_file_path" ]]; then
	echo "Found ${run_counter_file_path} from previous run to load run number."
	typeset -i run_counter=$(cat ${run_counter_file_path})
fi

previous_run_counter=$run_counter
let run_counter++
log_timestamp=$(date +%Y-%m-%d--%Hh%M)
log_file_name_prefix="${logs_path}/${run_counter}--${log_timestamp}--"

############################################### run steps  ############################################################


	(
		echo "====================================================================================================="
		echo "====================================================================================================="
		echo "===     Starting with run #${run_counter}..."
		echo "====================================================================================================="
		echo "====================================================================================================="
		ls -la ../../..
		echo "> echo \"${run_counter}\" | sudo -u mayadm tee ${run_counter_file_path}"
				echo "${run_counter}" | sudo -u mayadm tee ${run_counter_file_path}
		echo "> date"
				date
		echo "> dig +short txt ch whoami.cloudflare @1.0.0.1" #https://www.cyberciti.biz/faq/how-to-find-my-public-ip-address-from-command-line-on-a-linux/
				dig +short txt ch whoami.cloudflare @1.0.0.1
		echo "> ./maya_connect_internet_usb0.sh"
				./maya_connect_internet_usb0.sh
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [1] Cleaning working directory..."
		start_time=$(date +%s)
		sleep ${debug_step_sleep_s}s
		echo "> sudo -u mayadm git branch"
				sudo -u mayadm git branch
		echo "> sudo -u mayadm git status"
				sudo -u mayadm git status
		# add all tracket and all untracked files:
		echo "> sudo -u mayadm git add -u; sudo -u mayadm git add *"
				sudo -u mayadm git add -u; sudo -u mayadm git add *
		echo "> sudo -u mayadm git status"
				sudo -u mayadm git status
		echo "Checking if there are uncommitted changes..."
		(
			echo "> sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --"
					sudo -u mayadm git update-index --refresh && sudo -u mayadm git diff-index --quiet HEAD --
		) || (
			echo "********** WARNING **********"
			echo "Found uncommitted changes, staged it, will commit it to a new branch to not loose them..."
			sleep ${debug_step_sleep_s}s
			timestamp=$(date +%s)
			commit=$(sudo -u mayadm git rev-parse HEAD | cut -c0-7) #local variable inside ( ... )
			echo "> sudo -u mayadm git switch -c \"dev-${commit}-${timetamp}\""
					sudo -u mayadm git switch -c  "dev-${commit}-${timetamp}"
			echo "branch name is dev-${commit}-${timetamp}"
			echo '> sudo -u mayadm git commit -m "local changes automatically committed by maya_routine.sh"'
					sudo -u mayadm git commit -m "local changes automatically committed by maya_routine.sh"
		)

		echo "> sudo -u mayadm git reset --hard;"
				sudo -u mayadm git reset --hard
		echo "%%%%%%%%%%     [1] Cleaning working directory   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-1.log
	
	
	(
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [2] Loading source branch, pulling updates from origin..."
		start_time=$(date +%s)
		sleep ${debug_step_sleep_s}s
		echo "> sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"
				sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
		(
			echo "> sudo -u mayadm git pull" &&
					sudo -u mayadm git pull && \
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} successfully!"
		) || (
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} failed!"
			echo "Will build last commit on local branch ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"
			echo "> sudo -u mayadm git reset --hard"
					sudo -u mayadm git reset --hard
		)
		commit=$(sudo -u mayadm git rev-parse HEAD)
		echo "Last commit is:   ${commit}"
		echo "%%%%%%%%%%     [2] Loading source branch, pulling updates from origin   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-2.log
	
	commit_short=$(sudo -u mayadm git rev-parse HEAD | cut -c1-8)
	seconds_since_epoch=$(date -u +%s)
	LOCAL_WORKING_BRANCH="routine-${commit_short}-${seconds_since_epoch}"

	(
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [3] Switching to new branch and building sources..."
		start_time=$(date +%s)
		echo
		echo
		echo "%%%%%%%%%%     [3.1] Switching to new branch and updating git submodules..."
		sleep ${debug_step_sleep_s}s

		echo "Determining working branch name..."
		echo "branch name:   ${LOCAL_WORKING_BRANCH}"
		
		echo "Switching to working branch..."
		echo "> sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}"
				sudo -u mayadm git switch -c ${LOCAL_WORKING_BRANCH}
		
		echo "Updating git submodules..."
		(
			echo "> timeout 240m sudo -u mayadm git submodule update --init --recursive --depth 1"
					timeout 240m sudo -u mayadm git submodule update --init --recursive --depth 1
		) && echo "successfully updated git submodules" || echo "git submodul update FAILED!"
		(
			echo "%%%%%%%%%%     [3.2] Creating project and building sources (incremental build)..."
			sleep ${debug_step_sleep_s}s
			(
				echo "> timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh" &&
						timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh" &&
						timeout 60m sudo -u mayadm ./ub-unix-build.sh
			) && (
				echo "Build successfully finished! (incremental build)" &&
				echo "%%%%%%%%%%     [3.3] Skipped (clean build)" &&
				echo "%%%%%%%%%%     [3.4] Skipped (fallback clean build)"
			)
		) ||
		
		(
			echo "%%%%%%%%%%     [3.3] Creating project and building sources (clean build)..."
			sleep ${debug_step_sleep_s}s
			(
				echo "> timeout 60m sudo -u mayadm ./uc-unix-clean.sh" &&
						timeout 60m sudo -u mayadm ./uc-unix-clean.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh" &&
						timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh" &&
						timeout 60m sudo -u mayadm ./ub-unix-build.sh
			) && (
				echo "Build successfully finished! (clean build)" &&
				echo "%%%%%%%%%%     [3.4] Skipped (fallback clean build)"
			)
		) ||
		
		(
			echo "%%%%%%%%%%     [3.4] Creating project and building sources (fallback clean build)..."
			sleep ${debug_step_sleep_s}s
			(
				echo "> sudo -u mayadm git add * && sudo -u mayadm git add -u && sudo -u mayadm git fetch origin && sudo -u mayadm git reset --hard origin/${BRANCH_TO_LOAD_AS_FALLBACK_BRANCH}" &&
						sudo -u mayadm git add * && sudo -u mayadm git add -u && sudo -u mayadm git fetch origin && sudo -u mayadm git reset --hard origin/${BRANCH_TO_LOAD_AS_FALLBACK_BRANCH}
			) && (
				echo "> timeout 60m sudo -u mayadm ./uc-unix-clean.sh" &&
						timeout 60m sudo -u mayadm ./uc-unix-clean.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh" &&
						timeout 60m sudo -u mayadm ./ucp-unix-create-project.sh
			) && (
				echo "> timeout 60m sudo -u mayadm ./ub-unix-build.sh" &&
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
		)
		echo "%%%%%%%%%%     [3] Switching to new branch and building sources   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-3.log
	
	
	(
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [4] Run the executable..."
		start_time=$(date +%s)
		sleep ${debug_step_sleep_s}s

#this does not work due to locality of variables:

###		if [ "$build_success" = true ] ; then
###			#######add timeout here when sleep mode implies valve shutdown.
###			echo "> sudo -u mayadm ./ur-unix-run.sh"; \
###					sudo -u mayadm ./ur-unix-run.sh 
###			########use timeout XXXs ./run # 4 hours or so...
###		else
###			echo "FATAL! BUILD FAILED IN PREVIOUS STEP. WILL NOT EXECUTE!"
###		fi
		echo ">Please note:  Even if all builds failed, the last built executable is still there and will be started!"
		echo "> sudo -u mayadm ./ur-unix-run.sh"
				sudo -u mayadm ./ur-unix-run.sh
		echo "%%%%%%%%%%     [4] Run the executable   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-4.log
	
	(
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [5] Cleaning git working directory..."
		start_time=$(date +%s)
		sleep ${debug_step_sleep_s}s
		echo "> sudo -u mayadm git add -u"
				sudo -u mayadm git add -u
		echo "> sudo -u mayadm git add *"
				sudo -u mayadm git add *
		echo "> sudo -u mayadm git status"
				sudo -u mayadm git status
		echo "> sudo -u mayadm git reset --hard"
				sudo -u mayadm git reset --hard
		echo "Deleting local working branch..."
		echo "> sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"
				sudo -u mayadm git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
		echo "> sudo -u mayadm git branch -D ${LOCAL_WORKING_BRANCH}"
				sudo -u mayadm git branch -D ${LOCAL_WORKING_BRANCH}
		
		#to delete all branches matching specific pattern:
delete_branches=false
		if [ "$delete_branches" = true ] ; then
			echo "Deleting all branches matching a specific pattern..."
			echo "> git branch | grep routine-* | while read line"
					git branch | grep routine-* | while read line
			do
				echo "Delte branch: ${line}"
				echo "> git branch -D ${line}"
						git branch -D ${line}
			done
		fi
		echo "Finished run ${run_counter}!"
		date
		echo "%%%%%%%%%%     [5] Cleaning git working directory   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-5.log
	
	echo "> sudo -u mayadm cp -n ${logs_path} ../artifacts/ --recursive"
			sudo -u mayadm cp -n ${logs_path} ../artifacts/ --recursive # -n do not copy if file already present
			# if you copy inside a tee-tracked block, in most cases an empty file wil be copied, commited and the changes will not be copied on the next run.
	(
		echo "====================================================================================================="
		echo "%%%%%%%%%%     [6] Uploading logs and go sleeping..."
		start_time=$(date +%s)
		sleep ${debug_step_sleep_s}s
		echo "> sudo -u mayadm git checkout artifacts"
				sudo -u mayadm git checkout artifacts
		echo "> sudo -u mayadm git pull"
				sudo -u mayadm git pull
		echo "> sudo -u mayadm git merge ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}"
				sudo -u mayadm git merge ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}
		#sudo -u mayadm git add -u
		echo "> sudo -u mayadm git add \"../artifacts/logs/${run_counter}-*.log\" -f"
				sudo -u mayadm git add "../artifacts/logs/${run_counter}-*.log" -f
		echo "> sudo -u mayadm git add \"../artifacts/logs/${previous_run_counter}-*.log\" -f"
				sudo -u mayadm git add "../artifacts/logs/${previous_run_counter}-*.log" -f
		echo "> sudo -u mayadm git commit -m \"logs at ${log_timestamp}\""
				sudo -u mayadm git commit -m "logs at ${log_timestamp}"
		echo "> sudo -u mayadm git push"
				sudo -u mayadm git push
		echo "> sudo -u mayadm git checkout maya-productive"
				sudo -u mayadm git checkout maya-productive
		echo "> sudo -u mayadm git push"
				sudo -u mayadm git push


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
		echo "%%%%%%%%%%     [6] Uploading logs and go sleeping   ...DONE!"
		end_time=$(date +%s)
		echo "elapsed time:   $(( end_time - start_time ))s"
		echo "====================================================================================================="
	) 2>&1 | sudo -u mayadm tee ${log_file_name_prefix}-6.log


