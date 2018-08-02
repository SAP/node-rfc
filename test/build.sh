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

# Prerequisites:
# nvm install $lts
# npm -g install npm yarn

declare -a LTS_BUILD=("6.14.1" "8.11.3" "10.8.0")
declare -a LTS_TEST=("6.9.0" "6.14.3" "8.9.0" "8.11.3" "10.0.0" "10.8.0")

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

BUILD_LOG="test/pass-$osext.log"
rm $BUILD_LOG

#
# Build
#

if [ "$1" != "test" ]; then

    rm -rf lib/binding/$osext-*

    for lts in "${LTS_BUILD[@]}"
    do
        printf "\n============= building: $lts =============\n"
        nvm use $lts
        node-pre-gyp configure build package && printf "build: node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)')\n" >> $BUILD_LOG
    done

    npm run build

fi

#
# Test
#
for lts in "${LTS_TEST[@]}"
do
    nvm use $lts
    printf "\n============= testing: $lts =============\n"
    npm run test
    if [ $? == 0 ]; then
        test="pass :"
    else
        test="fail $?:"
    fi
    printf $test  >> $BUILD_LOG && printf " node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)') napi:$(node -e 'console.log(process.versions.napi)')\n" >> $BUILD_LOG
done
