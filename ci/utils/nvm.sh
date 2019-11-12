# NodeJS Version Manager

if [ -z "$NODEJS_HOME" ]; then
    printf "\nNODEJS_HOME env variable, for nodejs releases root folder not set\n"
    return 1
fi

if ! [ -d "$NODEJS_HOME" ]; then
    printf "\nNODEJS_HOME env variable points to non-existing directory: $NODEJS_HOME\n"
    return 1
fi

if [[ -z $1 || ! "$1" =~ ^(ls|use|remove|lsr|latest)$ ]]; then
    printf "Options:\n"
    printf "  nvm use [<version>]   download if not already and activate <version>\n"
    printf "  nvm remove <version>  remove <version>\n"
    printf "  nvm ls                show installed versions\n"
    printf "  nvm lsr [<version>]   show all nodejs versions\n"
    printf "  nvm latest [update]   show latest nodejs versions, update alias/version mappings\n"
    return 2
fi

# Remove NodeJS from PATH
clear_node_path () {
    NODEPATH=""
    while read -d ':' p; do
    if ! [[ $p =~ $NODEJS_HOME ]]; then
        NODEPATH=$NODEPATH:$p
    fi
    done <<< "$PATH:"
    PATH="${NODEPATH:1}" # chop first ":"
    unset NODEPATH
}

# Add/update NodeJS in PATH
set_node_path () {
    # check version parameter
    if [[ -z $1 ]]; then
        printf "Version missing\n"
        return 1
    fi

    # set NODEJS version path
    NODEPATH=""
    replaced=false
    while read -d ':' p; do
        if [[ $p =~ $NODEJS_HOME ]]; then
            if [ "$arch" = "win-x64" ]; then
                NODEPATH=$NODEPATH$:$NODEJS_HOME\\$1
            else
                NODEPATH=$NODEPATH$:$NODEJS_HOME/$1/bin
            fi
            replaced=true
        elif [ "$p" != "" ]; then
            NODEPATH=$NODEPATH:$p
        fi
    done <<< "$PATH:"
    if [ "$replaced" = false ]; then
        if [ "$arch" = "win-x64" ]; then
            NODEPATH=$NODEPATH$:$NODEJS_HOME\\$1
        else
            NODEPATH=$NODEPATH$:$NODEJS_HOME/$1/bin
        fi
    fi
    PATH="${NODEPATH:1}" # chop first ":"
    unset NODEPATH
}

set_version_prefix () {
    if [[ -z "$1" ]]; then
        printf "Version argument missing\n"
        return 4
    fi
    # add "v" prefix if missing
    first_char="$(printf '%s' "$1" | cut -c1)"
    if [ "$first_char" = l ]; then
        NVMVERSION=$(grep -E -m1 "$1" $NODEJS_HOME/latest-versions | grep -E -o 'v([0-9][0-9]?\.){2}[0-9][0-9]?')
    elif [ "$first_char" = v ]; then
        NVMVERSION=$1
    else
        NVMVERSION=v$1
    fi
}


platform=`uname`
arch=""

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


if [ "$1" = "lsr" ]; then
    curl -s http://nodejs.org/dist/ -o - | grep -E 'v[0-9].*|latest-*' | sed -e 's/.*node-//' -e 's/\.tar\.gz.*//' -e 's/<[^>]*>//' -e 's/\/<[^>]*>.*//'
    return
fi

if command -v node >/dev/null; then
    ACTIVE_VERSION=`node -v`
else
    ACTIVE_VERSION=""
fi

if [ "$1" = "remove" ]; then
    set_version_prefix $2
    if [ ! -d $NODEJS_HOME/$NVMVERSION ]; then
        printf "Version not found, not removed: $NVMVERSION\n"
        return 4
    fi
    rm -Rf $NODEJS_HOME/$NVMVERSION
    printf "nodejs version removed: $NVMVERSION\n"
    if [ "$NVMVERSION" = "$ACTIVE_VERSION" ]; then
        printf "active version unset: $ACTIVE_VERSION\n"
        clear_node_path
    fi
    return 0
fi

if [ "$1" = "ls" ]; then
    (cd $NODEJS_HOME &&
        for d in */ ; do
            v=${d%?}
            if [ "$ACTIVE_VERSION" = "$v" ]; then
                v="$v*"
            fi
            printf "$v\n"

        done)
    return
fi

if [ "$1" = "use" ]; then
    if [ -z "$2" ]; then
        printf "nodejs: `node -v` npm: `npm -v`\n"
        return 4
    fi

    # add "v" prefix if missing
    set_version_prefix $2

    # check if installed
    if [ ! -d $NODEJS_HOME/$NVMVERSION ]; then
        if [ "$arch" = "win-x64" ]; then
            # Windows installation
            (cd $NODEJS_HOME &&
                curl -OJ https://nodejs.org/dist/$NVMVERSION/node-$NVMVERSION-$arch.zip &&
                curl -OJ https://nodejs.org/download/release/$NVMVERSION/node-$NVMVERSION-headers.tar.gz &&
                echo unpacking: node-$NVMVERSION-$arch.zip
                winrar x -ibck node-$NVMVERSION-$arch.zip \*\.\* &&
                echo unpacking: node-$NVMVERSION-headers.tar.gz &&
                tar -xzf node-$NVMVERSION-headers.tar.gz &&
                mv node-$NVMVERSION-$arch $NVMVERSION &&
                mv node-$NVMVERSION/include $NVMVERSION/. &&
                rm -rf node-$NVMVERSION)
        else
            # Linux/Darwin installation
            (cd $NODEJS_HOME &&
                curl -OJ https://nodejs.org/dist/$NVMVERSION/node-$NVMVERSION-$arch.tar.gz &&
                tar -xzf node-$NVMVERSION-$arch.tar.gz &&
                mv node-$NVMVERSION-$arch $NVMVERSION)
        fi
        rm -f $NODEJS_HOME/node-$NVMVERSION-$arch.*
        rm -f $NODEJS_HOME/node-$NVMVERSION-headers.tar.gz

        if [ ! -d $NODEJS_HOME/$NVMVERSION ]; then
            printf "not found: $NVMVERSION\n"
            return 404
        fi

        set_node_path $NVMVERSION

        npm -g install npm
    else
        set_node_path $NVMVERSION
    fi
    unset NVMVERSION
    printf "nodejs: `node -v` npm: `npm -v`\n"
    export NODEJS_VERSION=`node -v`
    return 0
fi

# https://stackoverflow.com/questions/14093452/grep-only-the-first-match-and-stop
# https://stackoverflow.com/questions/16675179/how-to-use-sed-to-extract-substring
# https://stackoverflow.com/questions/16703647/why-curl-return-and-error-23-failed-writing-body


if [ "$1" = "latest" ]; then
    if ! [ -z "$2" ]; then
        if [ "$2" = "update" ]; then
            # get latest aliases
            # declare -A LATEST
            rm -Rf $NODEJS_HOME/latest-versions && touch $NODEJS_HOME/latest-versions
            rm -Rf $NODEJS_HOME/latest && curl -JLsN https://nodejs.org/dist/ | grep -E 'latest-|latest/' |  cut -d '"' -f2 > $NODEJS_HOME/latest
            while read ALIAS; do
                ALIAS="${ALIAS//\/}"
                VERSION=$(curl -JLsN "https://nodejs.org/dist/$ALIAS" | grep -E -m1 -o 'v([0-9][0-9]?\.){2}[0-9][0-9]?' | head -1 )
                #LATEST[$ALIAS]=$VERSION
                #printf "$ALIAS ${LATEST[$ALIAS]}\n" >> $NODEJS_HOME/latest-versions
                printf "%-16s %9s\n" $ALIAS $VERSION >> $NODEJS_HOME/latest-versions
            done <$NODEJS_HOME/latest
            rm -Rf $NODEJS_HOME/latest
        else
            printf "nvm latest option not supported: $2\n"
            return 6
        fi
    fi
    cat $NODEJS_HOME/latest-versions
else
    printf "\nOption not supported: $1\n"
    return 6
fi



# get version for alias
# curl -JLs https://nodejs.org/dist/latest-dubnium | grep -E -m1 -o 'v([0-9][0-9]?\.){2}[0-9][0-9]?' | head -1
### # get alias/version mapping
### declare -A LATEST
### while read ALIAS; do
###     ALIAS="${ALIAS//\/}"
###     VERSION=$(curl -JLsN "https://nodejs.org/dist/$ALIAS" | grep -E -m1 -o 'v([0-9][0-9]?\.){2}[0-9][0-9]?' | head -1 )
###     #printf "$ALIAS $VERSION\n"
###     LATEST[$ALIAS]=$VERSION
###     printf "$ALIAS ${LATEST[$ALIAS]}\n"
### done < <(curl -JLsN "https://nodejs.org/dist/" | grep -E 'latest-|latest/' |  cut -d '"' -f2)
