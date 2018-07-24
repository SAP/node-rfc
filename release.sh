# release build script
#
# Prerequisites:
# 1. node-rfc-release, at the same level as node-rfc
# 2. Python 2.7, required by node-gyp
#
# Run:
# source release.sh (bash release.sh)
#

# https://nodejs.org/en/download/releases/

# abi required as long node < 6.14.2 support needed
# https://github.com/nodejs/node-addon-api/issues/310#issuecomment-406659222

declare -a LTS_BUILD=("6.9.0" "6.14.2" "10.0.0")
declare -a LTS_TEST=("6.14.3" "8.11.3" "10.7.0")

version=`cat ./VERSION` 

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

rm -rf build/stage

for lts in "${LTS_BUILD[@]}"
do
    nvm install $lts && nvm use $lts && npm -g i npm

    rm -rf node_modules build/Release
    npm install
    # abi=`node --eval "console.log(require('node-abi').getAbi())"`
    npm run build && \
    node-pre-gyp clean configure build && \
    node-pre-gyp testbinary package reveal

    npm run test
done

for lts in "${LTS_TEST[@]}"
do
    nvm install $lts && nvm use $lts && npm -g i npm
    npm run test
done

