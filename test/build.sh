# release build script
#
# Prerequisites:
# 1. node-rfc-release, at the same level as node-rfc
# 2. Python 2.7, required by node-gyp
#
# Run:
# source build.sh (bash build.sh)
#

# Releases
# lts: https://github.com/nodejs/Release
# abi: https://nodejs.org/en/download/releases/

# abi required as long node < 6.14.2 support needed
# https://github.com/nodejs/node-addon-api/issues/310#issuecomment-406659222

# Prerequisites:
# nvm install $lts
# npm -g install npm yarn

# https://github.com/nodejs/node-addon-api/issues/387
declare -a LTS_BUILD=("8.9.0" "10.13.0" "11.0.0" "12.0.0")
#declare -a LTS_TEST=("8.9.0" "8.16.0" "10.13.0" "10.15.3" "11.0.0" "11.15.0" "12.3.0")
declare -a LTS_TEST=("8.16.0" "10.15.3" "11.15.0" "12.3.0")

version=`cat ./VERSION`

platform=`uname`

if [ "$platform" == "MSYS_NT-6.1-WOW" ]; then
	osext="win32"
elif [ "$platform" == "Linux" ]; then
	osext="linux"
elif [ "$platform" == "Darwin" ]; then
	osext="darwin"
else
	printf "\nPlatform not supported: $platform\n"
    return 1
fi

BUILD_LOG="test/pass-$osext.log"
rm $BUILD_LOG
touch $BUILD_LOG

NPMRELEASE="release/$version"
mkdir -p $NPMRELEASE


#
# Client
#

if [ "$1" != "test" ]; then

    rm -rf $NPMRELEASE/rfc-$osext*

    rm -rf lib/binding/$osext-*

    for lts in "${LTS_BUILD[@]}"
    do
        printf "\n============= building: $lts =============\n"
        nvm use $lts
        if [ $? -ne 0 ]; then
            exit
        fi

        if [ "$osext" == "darwin" ]; then
            # https://github.com/nodejs/node-gyp/issues/1564
            env CXXFLAGS="-mmacosx-version-min=10.9" node-pre-gyp clean configure build package && printf "build: node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)')\n" >> $BUILD_LOG
        else
            node-pre-gyp clean configure build package && printf "build: node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)')\n" >> $BUILD_LOG
        fi

        for filename in build/stage/$version/*.tar.gz; do
            mv $filename $NPMRELEASE/.
        done
    done

    #
    # Wrapper
    #

    npm run wrapper
    #npm run examples
fi

#
# Test
#

if [ "$1" != "build" ]; then

    for lts in "${LTS_TEST[@]}"
    do
        nvm use $lts
        if [ $? -ne 0 ]; then
            exit
        fi
        printf "\n============= testing: $lts =============\n"
        source ./test/test.sh
        if [ $? == 0 ]; then
            test="pass :"
        else
            test="fail $?:"
        fi
        printf $test  >> $BUILD_LOG && printf " node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)') napi:$(node -e 'console.log(process.versions.napi)')\n" >> $BUILD_LOG
    done

fi

cat $BUILD_LOG
