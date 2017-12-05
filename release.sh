# release build script

release_output="../node-rfc-release"

if [ "$(expr substr $(uname -s) 1 4)" == "MSYS" ]; then

    rm $release_output/*win32-x64.tar.gz

    nvm use 4.8.6
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v46-win32-x64.tar.gz build/win32_x64/rfc.node

    nvm use 6.12.0
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v48-win32-x64.tar.gz build/win32_x64/rfc.node

    nvm use 8.9.1
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v57-win32-x64.tar.gz build/win32_x64/rfc.node

elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then

    rm $release_output/*linux-x64.tar.gz

    export NVM_DIR="$HOME/.nvm"
    [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm

    nvm use 4
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v46-linux-x64.tar.gz  build/linux_x64/rfc.node

    nvm use 6
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v48-linux-x64.tar.gz  build/linux_x64/rfc.node

    nvm use 8
    node-pre-gyp clean configure
    node-pre-gyp build
    tar -czvf $release_output/rfc-v0.1.13-node-v57-linux-x64.tar.gz  build/linux_x64/rfc.node
fi
