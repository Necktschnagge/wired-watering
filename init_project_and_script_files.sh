#!/bin/bash

#+++PROJECT-NAME+++
project_name="Maya_Commander"
sources_url="https:\/\/github.com\/Necktschnagge\/automatic-watering"

sed -i -E "s/___template___project___name___/${project_name}/g" script/*.sh script/*.ps1 script/*.bat CMakeLists.txt azure-pipelines.yml

sed -i -E "s/___template___project___url___/${sources_url}/g" CMakeLists.txt

git update-index --chmod +x script/*.sh
git reset --mixed
git add *
