#!/bin/bash

LOCAL_WORKING_BRANCH="productive-clone"

BRANCH_TO_LOAD_AS_WORKING_BRANCH="productive"
BRANCH_TO_LOAD_AS_FALLBACK_BRANCH="productive-fallback"


run_counter=0


while true
do
	let run_counter++
	(
		echo "Starting with run #${run_counter}..."; \
		echo "Cleaning working directory..."; \
		git add -u; git add *; \
		git status; \
		git reset --hard; \
		echo "Cleaning working directory   ...DONE!"; \
		echo "Loading and updating working branch..."; \
		git checkout ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}; \
		(
			git pull && echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} successfully!"
		) || 
		(
			echo "Pulling ${BRANCH_TO_LOAD_AS_WORKING_BRANCH} failed! Resetting to last commit on local branch ${BRANCH_TO_LOAD_AS_WORKING_BRANCH}" && git git reset --hard
		) \
	) | tee logs/01.log
	echo "Switching to working branch..."
	git switch -c ${LOCAL_WORKING_BRANCH}
	
	
	
	echo "$Finished run #{run_counter}!"
done
