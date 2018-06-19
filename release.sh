# release build script
#
# Prerequisites:
# 1. node-rfc-release, at the same level as node-rfc
# 2. node-pre-gyp must be installed globally on windows
# 3. Python 2.7, required by node-gyp
# 4. Version updated in VERSION and package.json
#
# Run:
# source release.sh (bash release.sh)
#
# Ref: https://nodejs.org/en/download/releases/

version=`cat ./VERSION` 

declare -a LTS_VERSIONS=("6.14.3" "8.11.3" "10.4.1")

if [ "$(expr substr $(uname -s) 1 4)" == "MSYS" ]; then
	osext="win32"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	osext="linux"
else
    platform="$(expr substr $(uname -s) 1 10)"
	printf "\nPlatform not supported: $platform\n"
	exit 1
fi

printf "\nPlatform: $osext\n"

for lts in "${LTS_VERSIONS[@]}"
do
    nvm install $lts
    nvm use $lts
    rm -rf node_modules
    npm install
    abi=`node --eval "console.log(require('node-abi').getAbi())"`
    npm run build
    node-pre-gyp clean configure build && \
    node-pre-gyp testbinary && node-pre-gyp package && \
    npm run test && node-pre-gyp reveal
done

