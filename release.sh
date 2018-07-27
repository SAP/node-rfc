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

declare -a LTS_BUILD=("6.14.1" "8.11.3" "10.7.0")
declare -a LTS_TEST=("6.9.0" "6.14.3" "8.9.0" "8.11.3" "10.0.0" "10.7.0")

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

DEP="node-pre-gyp node-addon-api bluebird @types/bluebird"
# required for testing
DEPDEVTEST="@babel/core @types/node compare-versions decimal.js mocha should typescript"
# required for development and testing
DEPDEV="@babel/core @types/node compare-versions decimal.js eslint eslint-plugin-mocha mocha prettier-eslint should typescript"  

#
# Build
#
if [ "$1" != "test" ]; then

    rm -rf lib/binding/$osext-*
    BUILD_LOG="test/build-$osext.log"
    rm $BUILD_LOG
    for lts in "${LTS_BUILD[@]}"
    do
        printf "\n>>> build $lts >>>\n"
        nvm use $lts
        rm -rf node_modules
        yarn remove $DEP $DEPDEVTEST && yarn add $DEP && yarn add --dev $DEPDEVTEST && node-pre-gyp configure rebuild
    done

    yarn run build

    #
    # Package
    #
    rm -rf build/stage/$version/rfc-$osext-*
    for lts in "${LTS_BUILD[@]}"
    do
        printf "\>>> package $lts >>>\n"
        nvm use $lts
        node-pre-gyp package testpackage reveal
    done

fi

#
# Test
#

lts="8.11.3"
#for lts in "${LTS_TEST[@]}"
#do
    printf "\n>>> test $lts >>>\n"
    nvm use $lts
    rm -rf node_modules 
    yarn remove $DEP $DEPDEVTEST && yarn add $DEP && yarn add --dev $DEPDEVTEST && \
    yarn run test &$ printf "node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)') napi:$(node -e 'console.log(process.versions.napi)')" >> $BUILD_LOG
#done

# cleanup
rm -rf node_modules build/node_modules build/release

yarn remove $DEP $DEPDEVTEST && yarn add $DEP && yarn add --dev $DEPDEV