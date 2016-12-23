#!/bin/bash

# CONST
IGNORETEST=false
NODE_VERSIONS=(4.6.0 6.9.1)

# PARAM
if [ "$1" == "-i" ]; then
	IGNORETEST=true
fi

# VAR
nvm=""
nvm > /dev/null 2>&1
if [ $? -eq 0 ]; then
	nvm="nvm use"
else

	nodist > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		echo "Using nodist"
		nvm=nodist
	fi
fi
if [ "$nvm" == "" ]; then
	echo "Couldn't find node version manager"
	nv=$(node -v)
	if [ $? -ne 0 ]; then
		echo "Node is not installed. Aborting"
		exit 1
	fi
	found=false
	for v in ${NODE_VERSIONS[@]}; do
		if [ "v$v" == "$nv" ]; then
			found=true
		fi
	done
	if $found; then
		echo "Going to build only for installed version: " 
	else
		echo "Installed version $nv is not in targets: Aborting"
		exit 1
	fi
else
	echo "Using $nvm as node version manager to install versions: ${NODE_VERSIONS[@]}"
fi


# CODE
#TODO: checks: g++>4.8

echo "Checking environment"
if [ "$SAPNWRFC_HOME" == "" ]; then
	echo "SAPNWRFC_HOME is not set! Aborting"
	exit 1
fi
if [ "$NODE_PRE_GYP_GITHUB_TOKEN" == "" ]; then
	echo "NODE_PRE_GYP_GITHUB_TOKEN is not set! Aborting"
	exit 1
fi

echo "Updating repository"
git status | grep -q "nothing to commit, working directory clean" 2>/dev/null
if [ $? -ne 0 ]; then
	read -p "You have uncommited changes. Continue? [y/N]: " -n 1 continue
	if [[ "$continue" != [yY] ]]; then
		exit 1
	fi
	echo
	git checkout -- .
fi
git pull
#repolink=$(cat package.json | grep host | sed -r 's/.*"host": "(.*)\/releases.*/\1.git/')
#git clone --depth=10 $repolink
if [ $? -ne 0 ]; then
	echo "Failed updating the repository. Aborting"
	exit 1
fi

function compileAndTest {
	nv=$1
	if [ "$nvm" != "" ]; then
		echo "Switchig to node version: $nv"
		eval $nvm $nv > /dev/null
		if [ $? -ne 0 ]; then
			echo "Encountered error on switching version. Aborting"
			return 1
		fi
	fi
	node -v | grep -q "$nv\$" > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		if [ "$nvm" != "" ]; then
			echo "Failed switching node version. Aborting"
		fi
		return 1
	fi

	echo "Installing dependencies and compiling"
	rm -R node_modules
	npm install --build-from-source

	echo "Installing patches"
	for i in module_mods/*.patch; do patch -N -p0 < $i; done

	echo "Executing tests"
	npm test
	localSuccess=$?
	if [ $localSuccess -ne 0 ] && ! $IGNORETEST; then
		echo "Tests for Version $nv failed: Not publishing"
		return 1
	fi
	echo "Publishing"
	./node_modules/.bin/node-pre-gyp package && ./node_modules/.bin/node-pre-gyp-github publish --release
	return $?
}

declare -A success
for v in ${NODE_VERSIONS[@]}; do
	success[$v]=false
	compileAndTest $v
	[ $? -eq 0 ] && success[$v]=true
done

echo "Script completed"
echo "--------------------------------------------------------------------"
echo "Version | Success"
for k in ${!success[@]}; do
	echo $k "|" ${success[$k]}
done
