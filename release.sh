# release build script
#
# Prerequisites:
# 1. node-rfc-release, at the same level as node-rfc
# 2. node-pre-gyp must be installed globally on windows
# 3. Pythn 2.7, required by node-gyp
#
# Run:
# source release.sh

release_output="../../node-rfc-release"
release_root="../node-rfc-release"
version=`cat ./VERSION` 

declare -a LTS_VERSIONS=("4.9.1" "6.13.1" "8.11.1")

if [ "$(expr substr $(uname -s) 1 4)" == "MSYS" ]; then

    rm $release_root/*win32-x64.tar.gz

    for lts in "${LTS_VERSIONS[@]}"
    do
        nvm install $lts
        nvm use $lts
        rm -rf node_modules
        npm install
        abi=`node --eval "console.log(require('node-abi').getAbi())"`
        # echo "nodejs $lts abi $abi ====================================="
        npm uninstall node-pre-gyp
        npm -g install node-pre-gyp
        node-pre-gyp clean configure
        node-pre-gyp build
        cd build
        tar -czvf $release_output/rfc-v${version}-node-v${abi}-win32-x64.tar.gz rfc
        cd ..
    done

elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then

    rm $release_root/*linux-x64.tar.gz

    for lts in "${LTS_VERSIONS[@]}"
    do
        nvm install $lts
        nvm use $lts
        rm -rf node_modules
        npm install
        abi=`node --eval "console.log(require('node-abi').getAbi())"`
        # echo "nodejs $lts abi $abi ====================================="
        node-pre-gyp clean configure
        node-pre-gyp build
        cd build
        tar -czvf $release_output/rfc-v${version}-node-v${abi}-linux-x64.tar.gz rfc
        cd ..
    done

fi

