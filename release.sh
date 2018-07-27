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
# npm -g install npm

declare -a LTS_BUILD=("6.9.0" "8.9.0" "10.0.0")
declare -a LTS_TEST=("6.9.0" "6.14.3" "8.9.0" "8.11.3" "10.0.0" "10.7.0")

version=`cat ./VERSION` 

if [ "$(expr substr $(uname -s) 1 4)" == "MSYS" ]; then
	osext="win32"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	osext="linux"
    rm -rf build/stage/$version/rfc-$osext-*
else
    platform="$(expr substr $(uname -s) 1 10)"
	printf "\nPlatform not supported: $platform\n"
	exit 1
fi

BUILD_LOG="test/build-$osext.log"

rm $BUILD_LOG

rm -rf lib/binding/$osext-*
for lts in "${LTS_BUILD[@]}"
do
    nvm use $lts
    yarn install --dev
    npm run build && node-pre-gyp configure build package 
done

for lts in "${LTS_TEST[@]}"
do
    nvm use $lts
    npm run test && node-pre-gyp testpackage reveal && \
    echo node: $(node -v) npm:$(npm -v) abi:$(node -e 'console.log(process.versions.modules)') napi:$(node -e 'console.log(process.versions.napi)') >> $BUILD_LOG
done

# cleanup
rm -rf build/node_modules build/release

