cd ..
Write-Output "Cleaning all files   (git clean -f -d -X)"
git clean -f -d -X
Write-Output "Init git submodules"
git submodule update --init --recursive
cd script
	