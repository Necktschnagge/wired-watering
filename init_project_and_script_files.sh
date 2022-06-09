#!/bin/bash

#+++PROJECT-NAME+++
project_name="My_Project_Name"
sources_url="https:\/\/github.com\/github_user\/repo_id"

sed -i -E "s/___template___project___name___/${project_name}/g" script/*.sh script/*.ps1 script/*.bat CMakeLists.txt azure-pipelines.yml

sed -i -E "s/___template___project___url___/${sources_url}/g" CMakeLists.txt

git update-index --chmod +x script/*.sh
git reset --mixed
git add *
