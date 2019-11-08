# run from NodeJS folder
#  .. ci/utils/node-version-install.sh 10.17.0

if [ -z "$NODEJS_HOME" ]; then
	printf "\nNODEJS_HOME env variable, for nodejs releases root folder not set\n"
    return 1
fi

if [[ -z $1 || ! "$1" =~ ^(ls|use|remove)$ ]]; then
	printf "Options:\n"
    printf "  nvm use [<version>]\n"
    printf "  nvm remove <version>\n"
    printf "  nvm ls\n"
    return 2
fi

platform=`uname`
arch=""
export PATH_NODE_BASE=$PATH

if [ "$platform" = "Linux" ]; then
    arch="linux-x64"
elif [ "$platform" = "Darwin" ]; then
    arch="darwin-x64"
elif [ "$platform" = "MINGW64_NT-10.0-17763" ]; then
    arch="win-x64"
else
	printf "\nPlatform not supported: $platform\n"
    return 3
fi

if [ "$1" = "remove" ]; then
    if [ -z "$2" ]; then
        printf "nvm remove: version missing\n"
        return 4
    else
        rm -Rf $NODEJS_HOME/v$2
        printf "nodejs removed: $2\n"
        if [ "$NODE_VERSION" = "v$2" ]; then
            printf "active version unset: $2\n"
            export NODE_VERSION=""
            export PATH=$PATH_NODE_BASE
        fi
        return
    fi
fi

if [ "$1" = "ls" ]; then
    ls -ltr $NODEJS_HOME
    return
fi

if [ "$1" = "use" ]; then
    if [ -z "$2" ]; then
        printf "nodejs: `node -v` npm: `npm -v`\n"
        return 4
    fi

    if [ ! -d $NODEJS_HOME/v$2 ]; then
        pwd=`pwd`
        if [ "$arch" == "win-x64" ]; then
            # Windows installation
            cd $NODEJS_HOME &&
                curl -OJ https://nodejs.org/dist/v$2/node-v$2-$arch.zip &&
                curl -OJ https://nodejs.org/download/release/v$2/node-v$2-headers.tar.gz &&
                echo unpacking: node-v$2-$arch.zip
                winrar x -ibck node-v$2-$arch.zip \*\.\* &&
                echo unpacking: node-v$2-headers.tar.gz &&
                tar -xzf node-v$2-headers.tar.gz &&
                mv node-v$2-$arch v$2 &&
                mv node-v$2/include v$2/. &&
                rm -rf node-v$2
        else
            # Linux/Darwin installation
            cd $NODEJS_HOME &&
                curl -OJ https://nodejs.org/dist/v$2/node-v$2-$arch.tar.gz &&
                tar -xzf node-v$2-$arch.tar.gz &&
                mv node-v$2-$arch v$2
        fi
        cd $pwd
        unset pwd
        rm -f $NODEJS_HOME/node-v$2-$arch.zip
        rm -f $NODEJS_HOME/node-v$2-headers.tar.gz

        if [ ! -d $NODEJS_HOME/v$2 ]; then
            printf "not found: v$2\n"
            return 404
        fi
        if [ "$arch" == "win-x64" ]; then
            export PATH=$PATH_NODE_BASE:$NODEJS_HOME\\v$2
        else
            export PATH=$PATH_NODE_BASE:$NODEJS_HOME/v$2/bin
        fi
        npm -g install npm
    else
        if [ "$arch" == "win-x64" ]; then
            export PATH=$PATH_NODE_BASE:$NODEJS_HOME\\v$2
        else
            export PATH=$PATH_NODE_BASE:$NODEJS_HOME/v$2/bin
        fi
    fi
    export NODE_VERSION=v$2
    printf "nodejs: `node -v` npm: `npm -v`\n"

else
    printf "\nOption not supported: $1\n"
    return 6
fi



